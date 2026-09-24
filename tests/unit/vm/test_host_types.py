import contextlib
import io
import unittest
from pathlib import Path
from unittest.mock import patch

from panackelty import Code, PanackeltyError, VM, Value, build, verify_bytecode


class HostTypeRuntimeTests(unittest.TestCase):
    def test_oracle_matches_shared_host_type_conformance(self):
        case = Path(__file__).resolve().parents[2] / 'functional/cases/host_types'
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(build(case / 'main.panack')).run()
        self.assertEqual(output.getvalue(), (case / 'expected.stdout').read_text())

    def test_equal_ticks_do_not_make_different_opaque_types_equal(self):
        vm = VM({})
        duration = vm.builtin('duration_nanoseconds', [Value('Int', 0)])
        with patch('src.bootstrap.panackelty.time.clock_gettime_ns', return_value=0):
            instant = vm.builtin('instant_now', []).data[1][0]
        self.assertNotEqual(duration.data, instant.data)

    def test_clock_failure_is_a_structured_result(self):
        with patch('src.bootstrap.panackelty.time.clock_gettime_ns', side_effect=OSError('unavailable')):
            result = VM({}).builtin('instant_now', [])
        self.assertEqual(result.data[0], 'Error')
        self.assertEqual(result.data[1][0].data[0], 'ClockUnavailable')

    def test_clock_reading_is_effectful_in_verified_bytecode(self):
        code = {'main': Code('main', [], [('CALL', ('instant_now', 0)), ('RETURN', None)], pure=True)}
        with self.assertRaises(PanackeltyError):
            verify_bytecode(code)

    def test_path_lexical_boundaries_preserve_native_spelling(self):
        vm = VM({})
        for text, parent, filename in (
            ('a', '.', 'a'), ('a//b', 'a/', 'b'), ('/', '/', None),
            ('//', '//', None), ('///', '///', None), ('//host/a', '//host', 'a'),
            ('./', '.', '.'), ('a/../b/', 'a/..', 'b'),
        ):
            with self.subTest(path=text):
                path = vm.builtin('path_from_text', [Value('Str', text)]).data[1][0]
                directory = vm.builtin('path_directory', [path])
                self.assertEqual(vm.builtin('path_native_bytes', [directory]).data, parent.encode())
                result = vm.builtin('path_filename', [path])
                if filename is None:
                    self.assertEqual(result.data, ('None', []))
                else:
                    self.assertEqual(vm.builtin('path_native_bytes', result.data[1]).data, filename.encode())


if __name__ == '__main__':
    unittest.main()
