import json
import unittest
from pathlib import Path

from panackelty import Checker, Compiler, Parser, lex
from tests.unit.support import CompilerHarnessTestCase


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


FIXTURES = Path(__file__).resolve().parents[2] / "fixtures/compiler_contracts"


class SelfHostedEmitterTests(CompilerHarnessTestCase):
    maxDiff = None

    def test_shared_disassembly_matches_bootstrap(self):
        cases = json.loads((FIXTURES / "emitter/manifest.json").read_text())
        self.assertEqual(len(cases), 10)
        for case in cases:
            with self.subTest(case=case["name"]):
                source = (FIXTURES / (case["name"] + ".panack")).read_text(encoding="utf-8")
                expected = (FIXTURES / (case["name"] + ".disasm")).read_text(encoding="utf-8")
                actual = self.run_harness(
                    "compiler/emitter.panack",
                    "main(): Void { print(compile_source_disassembly(command_args()[0])) }",
                    [source],
                ).removesuffix("\n")
                self.assertEqual(actual, render_bootstrap(source))
                self.assertEqual(actual, expected)
