import contextlib
import io
import os
from pathlib import Path
import tempfile
import time
import unittest

from panackelty import Code, VM, Value, PanackeltyError, build, verify_bytecode

PROJECT = Path(__file__).resolve().parents[3]


class HostCapabilityTests(unittest.TestCase):
    def setUp(self):
        self.vm = VM({})
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def path(self, text):
        return self.vm.builtin('path_from_native', [Value('Bytes', os.fsencode(text))]).data[1][0]

    def duration(self, ns):
        return self.vm.builtin('duration_nanoseconds', [Value('Int', ns)])

    def process(self, script, *, data=b'', limit=1000000, timeout=3000000000, env=(), cwd=None):
        return self.vm.builtin('process_run', [self.path('/bin/sh'),
            Value('[Str]', [Value('Str', '-c'), Value('Str', script)]), Value('Bytes', data),
            self.path(cwd or self.root), Value('[Str]', [Value('Str', v) for v in env]),
            self.duration(timeout), Value('Nat', limit)])

    def assert_error(self, result, code):
        self.assertEqual(result.data[0], 'Error')
        self.assertEqual(result.data[1][0].data['code'].data, code)

    def test_oracle_shared_conformance(self):
        case = PROJECT / 'tests/functional/cases/host_capabilities'
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(build(case / 'main.panack')).run()
        self.assertEqual(output.getvalue(), (case / 'expected.stdout').read_text())

    def test_large_duplex_streams_and_early_stdin_close(self):
        result = self.process('dd if=/dev/zero bs=1024 count=128 2>/dev/null; '
                              'dd if=/dev/zero bs=1024 count=128 >&2 2>/dev/null; cat', data=b'x' * 131072)
        self.assertEqual(result.data[0], 'Ok')
        fields = result.data[1][0].data
        self.assertEqual(fields['stdout'].data, b'\0' * 131072 + b'x' * 131072)
        self.assertEqual(fields['stderr'].data, b'\0' * 131072)
        self.assertEqual(self.process('exit 0', data=b'x' * 131072).data[0], 'Ok')

    def test_environment_cwd_exit_signal_and_invalid_utf8(self):
        result = self.process('printf "$PANACK_HOST_TEST"; pwd; printf "\\377" >&2; exit 9', env=['PANACK_HOST_TEST=value'])
        fields = result.data[1][0].data
        self.assertEqual(fields['stdout'].data, b'value' + os.fsencode(self.root.resolve()) + b'\n')
        self.assertEqual(fields['exit_code'].data, 9)
        self.assertEqual(fields['stderr'].data, b'\xff')
        self.assert_error(self.vm.builtin('host_decode_utf8', [fields['stderr']]), 'invalid_utf8')
        self.assertEqual(self.process('kill -TERM $$').data[1][0].data['signal'].data, 15)
        self.assertNotIn('PANACK_HOST_TEST', os.environ)
        self.assert_error(self.process('true', env=['A=1', 'A=2']), 'invalid_environment')
        self.assert_error(self.process('true', cwd=self.root / 'missing'), 'launch_failed')

    def test_limits_and_timeouts_are_bounded(self):
        started = time.monotonic()
        self.assert_error(self.process('sleep 10 & wait', timeout=20000000), 'timeout')
        self.assertLess(time.monotonic() - started, 2)
        self.assert_error(self.process('printf x', limit=0), 'output_limit')
        self.assert_error(self.process('true', timeout=-1), 'negative_duration')
        self.assert_error(self.process('true', timeout=31536000000000001), 'out_of_range')
        self.assert_error(self.process('touch should-not-exist', timeout=0), 'timeout')
        self.assertFalse((self.root / 'should-not-exist').exists())

    def test_filesystem_symlinks_limits_and_explicit_cleanup(self):
        target = self.root / 'file'
        path = self.path(target)
        self.assertEqual(self.vm.builtin('fs_write', [path, Value('Bytes', b'ab')]).data[0], 'Ok')
        self.assert_error(self.vm.builtin('fs_read', [path, Value('Nat', 1)]), 'output_limit')
        link = self.root / 'link'
        link.symlink_to(target)
        metadata = self.vm.builtin('fs_metadata', [self.path(link)]).data[1][0].data
        self.assertEqual(metadata['kind'].data[0], 'SymbolicLink')
        self.assertEqual(self.vm.builtin('fs_read', [self.path(link), Value('Nat', 2)]).data[1][0].data, b'ab')
        self.vm.builtin('fs_remove_file', [self.path(link)])
        self.assertTrue(target.exists())
        self.assert_error(self.vm.builtin('fs_read', [self.path(self.root), Value('Nat', 0)]), 'not_regular_file')
        fifo = self.root / 'fifo'
        os.mkfifo(fifo)
        self.assert_error(self.vm.builtin('fs_read', [self.path(fifo), Value('Nat', 0)]), 'not_regular_file')
        first = self.vm.builtin('fs_temp_file', [self.path(self.root)]).data[1][0]
        second = self.vm.builtin('fs_temp_file', [self.path(self.root)]).data[1][0]
        self.assertNotEqual(first, second)
        self.assertEqual(os.stat(self.vm.builtin('path_native_bytes', [first]).data).st_mode & 0o777, 0o600)
        raw = os.fsencode(self.root) + b'/\xff'
        try:
            with open(raw, 'wb') as stream:
                stream.write(b'raw')
        except OSError:
            return  # Some supported host filesystems reject non-UTF-8 names.
        entries = self.vm.builtin('fs_list', [self.path(self.root)]).data[1][0].data
        self.assertIn(b'\xff', [self.vm.builtin('path_native_bytes', [p]).data for p in entries])
        self.assertEqual(self.vm.builtin('fs_read', [self.path(raw), Value('Nat', 3)]).data[1][0].data, b'raw')

    def test_sleep_waits_for_requested_duration(self):
        start = time.clock_gettime_ns(time.CLOCK_MONOTONIC)
        self.assertEqual(self.vm.builtin('host_sleep', [self.duration(1000000)]).data[0], 'Ok')
        self.assertGreaterEqual(time.clock_gettime_ns(time.CLOCK_MONOTONIC) - start, 1000000)

    def test_effects_are_rejected_by_verifier_and_operands_by_runtime(self):
        for name, count in [('fs_read', 2), ('fs_write', 2), ('fs_metadata', 1), ('fs_list', 1),
                            ('fs_create_directory', 1), ('fs_remove_file', 1), ('fs_remove_directory', 1),
                            ('fs_temp_file', 1), ('fs_temp_directory', 1), ('host_sleep', 1), ('process_run', 7)]:
            with self.subTest(name=name):
                code = {'main': Code('main', [], [('CONST', ('Nat', 0))] * count + [('CALL', (name, count)), ('RETURN', None)], pure=True)}
                with self.assertRaises(PanackeltyError):
                    verify_bytecode(code)
                with self.assertRaises(PanackeltyError):
                    self.vm.builtin(name, [Value('Nat', 0)] * count)


if __name__ == '__main__':
    unittest.main()
