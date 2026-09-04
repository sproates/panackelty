import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import Checker, Compiler, Parser, VM, build, lex


PROJECT = Path(__file__).resolve().parents[3]
COMPILER = PROJECT / "src/compiler"


def render_operand(op, arg):
    if op == "CONST":
        type_name, value = arg
        if type_name == "Bool":
            value = "true" if value else "false"
        elif type_name == "Void":
            return "Void"
        elif type_name == "Dec":
            value = str(value)
        return f"{type_name}:{value}"
    if op in {"LOAD", "STORE", "UNARY", "BINARY", "ITER_INIT", "FIELD_GET"}:
        return str(arg)
    if op in {"MAKE_ARRAY", "JUMP", "JUMP_FALSE", "CALL_VALUE"}:
        return str(arg)
    if op == "INTERPOLATE":
        return ",".join(arg)
    if op == "ITER_NEXT":
        return f"{arg[0]}|{arg[1]}|{arg[2]}"
    if op == "MAKE_RECORD":
        return f"{arg[0]}|{','.join(arg[1])}"
    if op == "MAKE_VARIANT":
        return f"{arg[0]}|{arg[1]}|{arg[2]}"
    if op == "MATCH_VARIANT":
        return f"{arg[0]}|{arg[1]}"
    if op == "CALL":
        return f"{arg[0]}|{arg[1]}"
    return ""


def render_bootstrap(source):
    program = Parser(lex(source)).parse()
    Checker(program).check()
    functions = Compiler(program).compile()
    lines = []
    for function in functions.values():
        purity = "pure" if function.pure else "impure"
        lines.append(
            f"FUNCTION|{function.name}|{purity}|{','.join(function.params)}"
        )
        for index, (op, arg) in enumerate(function.instructions):
            operand = render_operand(op, arg)
            lines.append(f"{index}|{op}" + (f"|{operand}" if operand else ""))
    return "\n".join(lines) + "\n"


class SelfHostedEmitterTests(unittest.TestCase):
    maxDiff = None

    def emit(self, source):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in (
                "types.panack",
                "lexer.panack",
                "parser.panack",
                "resolver.panack",
                "checker.panack",
                "purity.panack",
                "emitter.panack",
            ):
                (root / name).write_text(
                    (COMPILER / name).read_text(encoding="utf-8"),
                    encoding="utf-8",
                )
            main = root / "main.panack"
            source_expression = ' + "$" + '.join(
                json.dumps(part) for part in source.split("$")
            )
            main.write_text(
                'import "emitter.panack";\n'
                f"main(): Void {{ print(compile_source_disassembly({source_expression})); }}",
                encoding="utf-8",
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(main)).run()
            return output.getvalue().removesuffix("\n")

    def assert_differential(self, source):
        self.assertEqual(self.emit(source), render_bootstrap(source))

    def test_emits_scalars_calls_and_control_flow(self):
        source = """
pure choose(value: Nat): Nat {
  if value > 0 { value - 1 } else { 0 }
}
main(): Void { print(choose(2)); }
"""
        self.assert_differential(source)

    def test_emits_method_calls_as_receiver_first_calls(self):
        source = """
pure increment(value: Nat, amount: Nat): Nat { value + amount }
main(): Void {
  values: [Nat] = [40.increment(2)].append(43);
  mut answers: Map[Str,Nat] = map();
  answers = answers.put("answer", values[0]);
  print(answers.has("answer"));
  print(answers.get("answer"));
  print(set().add("answer").has("answer"));
  print(values.len());
}
"""
        self.assert_differential(source)

    def test_emits_if_without_else_with_balanced_void_paths(self):
        source = """
main(): Void {
  mut answer: Nat = 0;
  if true { answer = 42; };
  if false { 99 };
  print(answer);
}
"""
        self.assert_differential(source)

    def test_emits_loops_arrays_indexing_and_short_circuiting(self):
        source = """
pure sum(limit: Nat): Nat {
  mut total: Nat = 0;
  for value in 0..limit { total = total + value; }
  mut cursor: Nat = limit;
  while cursor > 0 { cursor = cursor - 1; }
  if limit == 0 || total >= limit { total } else { 0 }
}
main(): Void {
  values: [Nat] = [sum(4), 42];
  print(values[0]);
}
"""
        self.assert_differential(source)

    def test_emits_records_variants_matches_and_interpolation(self):
        source = """
record Pair { left: Nat, right: Nat }
enum Maybe { None, Some(Nat) }
pure unwrap(value: Maybe): Nat {
  match value { Some(item) => item, None() => 0 }
}
main(): Void {
  pair: Pair = Pair(20, 22);
  answer: Nat = unwrap(Some(pair.left + pair.right));
  print("answer ${answer}");
}
"""
        self.assert_differential(source)

    def test_emits_exact_decimals_and_escaped_strings(self):
        source = r'''
pure decimal(): Dec { 12.3400 }
main(): Void {
  message: Str = "line\ntext";
  print(message);
  print(decimal());
}
'''
        self.assert_differential(source)

    def test_emits_function_references_indirect_calls_map_and_reduce(self):
        source = """
pure twice(value: Nat): Nat { value * 2 }
pure total(accumulator: Nat, value: Nat): Nat { accumulator + value }
main(): Void {
  callback: PureFn[Nat,Nat] = @twice
  print(callback.call(3))
  print([1, 2].map(callback))
  print([1, 2].reduce(0, @total))
}
"""
        self.assert_differential(source)


if __name__ == "__main__":
    unittest.main()
