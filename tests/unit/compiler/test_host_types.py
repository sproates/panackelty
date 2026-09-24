import unittest

from panackelty import Checker, PanackeltyError, Parser, lex
from tests.unit.support import CompilerHarnessTestCase


DECLARATIONS = '''
enum Result[T,E] { Ok(T), Error(E) }
enum Option[T] { Some(T), None }
enum PathError { EmptyPath, PathContainsNul, PathNotUtf8, AbsolutePathAppend }
enum DurationError { FractionalNanosecond, ZeroDurationDivisor }
enum ClockError { ClockUnavailable }
record HostError { operation: Str, code: Str, native_code: Nat }
enum FileKind { RegularFile, Directory, SymbolicLink, OtherFile }
record FileMetadata { kind: FileKind, size: Nat }
record ProcessOutput { exit_code: Nat, signal: Nat, stdout: Bytes, stderr: Bytes }
'''


class HostTypeCheckerTests(CompilerHarnessTestCase):
    def check_both(self, source, valid):
        result = self.run_harness(
            'compiler/purity.panack',
            'main(): Void { print(check_source_frontend(command_args()[0])) }',
            [DECLARATIONS + source],
        ).strip()
        self.assertEqual(result == 'ok', valid, result)
        if valid:
            Checker(Parser(lex(DECLARATIONS + source)).parse()).check()
        else:
            with self.assertRaises(PanackeltyError):
                Checker(Parser(lex(DECLARATIONS + source)).parse()).check()

    def test_opaque_values_work_in_generics_records_and_pure_calls(self):
        self.check_both('''
record Box[T] { value: T }
pure identity[T](value: T): T { value }
pure duration(value: Int): Duration { duration_nanoseconds(value) }
main(): Void {
  path: Path = path_current()
  values: [Path] = [identity(path)]
  box: Box[Duration] = Box(duration(-1))
  current: Result[Instant,ClockError] = instant_now()
  match current {
    Error(e) => {},
    Ok(start) => {
      deadline: Instant = instant_add(start, box.value)
      elapsed: Duration = instant_difference(deadline, start)
      print(instant_before(deadline, start))
    }
  }
}
''', True)

    def test_host_capability_types_and_effects(self):
        self.check_both('\nmain(): Void {\n  contents: Result[Bytes,HostError] = fs_read(path_current(), 100)\n  entries: Result[[Path],HostError] = fs_list(path_current())\n  output: Result[ProcessOutput,HostError] = process_run(path_current(), [], bytes(), path_current(), [], duration_nanoseconds(1), 100)\n}\n', True)
        for source in (
            'main(): Void { x = fs_read("file", 10) }',
            'main(): Void { x = fs_write(path_current(), "text") }',
            'main(): Void { x = host_sleep(1) }',
            'pure bad(): Result[[Path],HostError] { fs_list(path_current()) } main(): Void {}',
            'pure bad(): Result[Unit,HostError] { host_sleep(duration_nanoseconds(0)) } main(): Void {}',
            'main(): Void { x = process_run(path_current(), [1], bytes(), path_current(), [], duration_nanoseconds(0), 10) }',
        ):
            with self.subTest(source=source):
                self.check_both(source, False)

    def test_invalid_types_forged_construction_and_impure_clock_are_rejected(self):
        for source in (
            'main(): Void { x: Path = "a" }',
            'main(): Void { x: Duration = 1 }',
            'main(): Void { x: Instant = 0 }',
            'main(): Void { x = Path("a") }',
            'main(): Void { x = Instant(0) }',
            'main(): Void { x = Duration(0) }',
            'main(): Void { x = path_current().value }',
            'main(): Void { x = duration_nanoseconds(1).value }',
            'main(): Void { x = instant_add(duration_nanoseconds(1), duration_nanoseconds(1)) }',
            'main(): Void { x = duration_ticks(path_current()) }',
            'main(): Void { x = path_append(path_current(), "a") }',
            'main(): Void { x = duration_from_seconds(0.5) }',
            'pure now(): Result[Instant,ClockError] { instant_now() } main(): Void {}',
            'record Path { raw: Str } main(): Void {}',
            'record Duration { raw: Int } main(): Void {}',
            'enum Instant { FakeInstant } main(): Void {}',
            'record Box[Path] { value: Path } main(): Void {}',
            'enum Box[Duration] { Wrap(Duration) } main(): Void {}',
            'pure f[Instant](x: Instant): Instant { x } main(): Void {}',
        ):
            with self.subTest(source=source):
                self.check_both(source, False)


if __name__ == '__main__':
    unittest.main()
