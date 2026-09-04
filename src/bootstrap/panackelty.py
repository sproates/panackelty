#!/usr/bin/env python3
from __future__ import annotations

import argparse
import decimal
import json
import os
import re
import sys
from dataclasses import dataclass
from fractions import Fraction
from pathlib import Path
from typing import Any

PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_STDLIB_ROOT = PROJECT_ROOT / "src" / "stdlib"

class PanackeltyError(Exception):
    pass


class PanackeltyProcessExit(Exception):
    def __init__(self, code: int):
        self.code = code


@dataclass(frozen=True)
class Token:
    kind: str
    text: str
    line: int
    col: int
    file: str = ""


def source_location(token: Token) -> str:
    prefix = f"{token.file}:" if token.file else ""
    return f"{prefix}{token.line}:{token.col}"


TOKEN_RE = re.compile(
    r"(?P<WS>[ \t\r]+)|(?P<COMMENT>//[^\n]*)|(?P<NL>\n)|"
    r"(?P<DEC>\d+\.\d+)|(?P<INT>\d+)|"
    r'(?P<STRING>"(?:\\.|[^"\\])*")|'
    r"(?P<FATARROW>=>)|(?P<RANGE>\.\.)|(?P<OP>==|!=|<=|>=|&&|\|\||[+\-*/%<>!])|"
    r"(?P<PUNCT>[(){}\[\].,;:=@])|(?P<IDENT>[A-Za-z_][A-Za-z0-9_]*)"
)


LINE_PREFIX_KEYWORDS = {"pure", "import", "type", "record", "enum", "mut",
                        "for", "while", "if", "match", "where", "in"}
LINE_CONTINUATION_KEYWORDS = {"else", "where", "in"}


def token_can_end_statement(token: Token) -> bool:
    return (
        (token.kind in {"IDENT", "INT", "DEC", "STRING"}
         and token.text not in LINE_PREFIX_KEYWORDS)
        or token.text in {")", "]", "}"}
    )


def token_can_start_statement(token: Token) -> bool:
    return (
        token.text not in LINE_CONTINUATION_KEYWORDS
        and (token.kind in {"IDENT", "INT", "DEC", "STRING"}
             or token.text in {"(", "["})
    )


def normalize_line_breaks(tokens: list[Token]) -> list[Token]:
    result: list[Token] = []
    soft_depth = 0
    brace_depth = 0
    brace_soft_depths: dict[int, int] = {}

    for index, token in enumerate(tokens):
        if token.kind == "NL":
            next_index = index + 1
            while next_index < len(tokens) and tokens[next_index].kind == "NL":
                next_index += 1
            baseline = brace_soft_depths.get(brace_depth, 0) if brace_depth else 0
            soft = soft_depth > baseline
            if (not soft and result and next_index < len(tokens)
                    and token_can_end_statement(result[-1])
                    and token_can_start_statement(tokens[next_index])):
                result.append(Token("PUNCT", ";", token.line, token.col, token.file))
            continue

        result.append(token)
        if token.text in {"(", "["}:
            soft_depth += 1
        elif token.text in {")", "]"} and soft_depth > 0:
            soft_depth -= 1
        elif token.text == "{":
            brace_depth += 1
            brace_soft_depths[brace_depth] = soft_depth
        elif token.text == "}" and brace_depth > 0:
            brace_depth -= 1

    return result


def lex(source: str, file: str = "") -> list[Token]:
    result: list[Token] = []
    pos = 0
    line = col = 1
    while pos < len(source):
        match = TOKEN_RE.match(source, pos)
        if not match:
            prefix = f"{file}:" if file else ""
            raise PanackeltyError(f"{prefix}{line}:{col}: unexpected character {source[pos]!r}")
        kind, text = match.lastgroup, match.group()
        if kind == "NL":
            result.append(Token("NL", text, line, col, file))
            line += 1
            col = 1
        else:
            if kind not in {"WS", "COMMENT"}:
                result.append(Token(kind or "", text, line, col, file))
            col += len(text)
        pos = match.end()
    result.append(Token("EOF", "", line, col, file))
    return normalize_line_breaks(result)


@dataclass
class Expr:
    kind: str
    value: Any = None
    args: tuple[Expr, ...] = ()
    token: Token | None = None


@dataclass
class Pattern:
    variant: str
    bindings: list[str]
    token: Token


@dataclass
class Let:
    name: str
    type_name: str
    value: Expr
    mutable: bool
    token: Token


@dataclass
class Assign:
    name: str
    value: Expr
    token: Token


@dataclass
class While:
    condition: Expr
    body: Block
    token: Token


@dataclass
class For:
    name: str
    iterable: Expr
    body: Block
    token: Token


@dataclass
class Block:
    statements: list[Let | Assign | While | For | Expr]
    result: Expr


@dataclass
class Function:
    name: str
    params: list[tuple[str, str]]
    return_type: str
    pure: bool
    body: Block
    token: Token


@dataclass
class TypeDef:
    name: str
    base: str
    guard: Expr
    token: Token


@dataclass
class RecordDef:
    name: str
    type_params: list[str]
    fields: list[tuple[str, str]]
    token: Token


@dataclass
class EnumDef:
    name: str
    type_params: list[str]
    variants: dict[str, list[str]]
    token: Token


@dataclass
class Program:
    imports: list[str]
    types: dict[str, TypeDef]
    records: dict[str, RecordDef]
    enums: dict[str, EnumDef]
    functions: dict[str, Function]


class Parser:
    PRECEDENCE = {"..": 0, "||": 1, "&&": 2, "==": 3, "!=": 3, "<": 4, "<=": 4,
                  ">": 4, ">=": 4, "+": 5, "-": 5, "*": 6, "/": 6, "%": 6}

    def __init__(self, tokens: list[Token]):
        self.tokens = tokens
        self.i = 0

    def peek(self, text: str | None = None) -> Token | bool:
        tok = self.tokens[self.i]
        return tok.text == text if text is not None else tok

    def take(self, text: str | None = None, kind: str | None = None) -> Token:
        tok = self.tokens[self.i]
        if text is not None and tok.text != text:
            self.fail(tok, f"expected {text!r}, found {tok.text!r}")
        if kind is not None and tok.kind != kind:
            self.fail(tok, f"expected {kind.lower()}, found {tok.text!r}")
        self.i += 1
        return tok

    @staticmethod
    def fail(tok: Token, message: str) -> None:
        raise PanackeltyError(f"{source_location(tok)}: {message}")

    def import_path(self) -> str:
        token = self.tokens[self.i]
        if token.kind == "STRING":
            self.i += 1
            return json.loads(token.text)
        if token.kind != "IDENT":
            self.fail(token, "expected import path")
        parts = [self.take(kind="IDENT").text]
        if not self.peek("/"):
            self.fail(self.tokens[self.i], "expected '/' in logical import path")
        while self.peek("/"):
            self.take()
            parts.append(self.take(kind="IDENT").text)
        if self.peek("."):
            self.take()
            suffix = self.take(kind="IDENT")
            if suffix.text != "panack":
                self.fail(suffix, "logical import extension must be .panack")
        return "/".join(parts)

    def parse(self) -> Program:
        types: dict[str, TypeDef] = {}
        imports: list[str] = []
        records: dict[str, RecordDef] = {}
        enums: dict[str, EnumDef] = {}
        functions: dict[str, Function] = {}
        while self.tokens[self.i].kind != "EOF":
            if self.peek(";"):
                self.take()
            elif self.peek("import"):
                self.take()
                imports.append(self.import_path())
                if self.peek(";"):
                    self.take()
                elif self.tokens[self.i].kind != "EOF":
                    self.fail(self.tokens[self.i], "expected line break or semicolon after import")
            elif self.peek("type"):
                item = self.type_def()
                if item.name in types:
                    self.fail(item.token, f"duplicate type {item.name}")
                types[item.name] = item
            elif self.peek("record"):
                item = self.record_def()
                if item.name in records:
                    self.fail(item.token, f"duplicate record {item.name}")
                records[item.name] = item
            elif self.peek("enum"):
                item = self.enum_def()
                if item.name in enums:
                    self.fail(item.token, f"duplicate enum {item.name}")
                enums[item.name] = item
            else:
                item = self.function()
                if item.name in functions:
                    self.fail(item.token, f"duplicate function {item.name}")
                functions[item.name] = item
        return Program(imports, types, records, enums, functions)

    def record_def(self) -> RecordDef:
        start = self.take("record")
        name = self.take(kind="IDENT").text
        type_params = self.generic_params()
        self.take("{")
        fields: list[tuple[str, str]] = []
        while not self.peek("}"):
            field = self.take(kind="IDENT").text
            self.take(":")
            fields.append((field, self.type_name()))
            if not self.peek(","):
                break
            self.take()
        self.take("}")
        return RecordDef(name, type_params, fields, start)

    def enum_def(self) -> EnumDef:
        start = self.take("enum")
        name = self.take(kind="IDENT").text
        type_params = self.generic_params()
        self.take("{")
        variants: dict[str, list[str]] = {}
        while not self.peek("}"):
            variant_token = self.take(kind="IDENT")
            payload: list[str] = []
            if self.peek("("):
                self.take()
                if not self.peek(")"):
                    while True:
                        payload.append(self.type_name())
                        if not self.peek(","):
                            break
                        self.take()
                self.take(")")
            if variant_token.text in variants:
                self.fail(variant_token, f"duplicate variant {variant_token.text}")
            variants[variant_token.text] = payload
            if not self.peek(","):
                break
            self.take()
        self.take("}")
        return EnumDef(name, type_params, variants, start)

    def generic_params(self) -> list[str]:
        params: list[str] = []
        if self.peek("["):
            self.take()
            while True:
                params.append(self.take(kind="IDENT").text)
                if not self.peek(","):
                    break
                self.take()
            self.take("]")
        return params

    def type_def(self) -> TypeDef:
        start = self.take("type")
        name = self.take(kind="IDENT").text
        self.take("=")
        base = self.take(kind="IDENT").text
        self.take("where")
        guard = self.expr()
        if self.peek(";"):
            self.take()
        elif self.tokens[self.i].kind != "EOF":
            self.fail(self.tokens[self.i], "expected line break or semicolon after type declaration")
        return TypeDef(name, base, guard, start)

    def function(self) -> Function:
        pure = False
        if self.peek("pure"):
            self.take()
            pure = True
        start = self.take(kind="IDENT")
        name = start.text
        self.take("(")
        params: list[tuple[str, str]] = []
        if not self.peek(")"):
            while True:
                param = self.take(kind="IDENT").text
                self.take(":")
                type_name = self.type_name()
                params.append((param, type_name))
                if not self.peek(","):
                    break
                self.take()
        self.take(")")
        self.take(":")
        return_type = self.type_name()
        return Function(name, params, return_type, pure, self.block(), start)

    def type_name(self) -> str:
        if self.peek("["):
            self.take()
            inner = self.type_name()
            self.take("]")
            return f"Array[{inner}]"
        name = self.take(kind="IDENT").text
        if self.peek("["):
            self.take()
            arguments: list[str] = []
            while True:
                arguments.append(self.type_name())
                if not self.peek(","):
                    break
                self.take()
            self.take("]")
            return f"{name}[{','.join(arguments)}]"
        return name

    def block(self) -> Block:
        self.take("{")
        statements: list[Let | Assign | While | For | Expr] = []
        while True:
            if self.peek("}"):
                end = self.take()
                return Block(statements, Expr("literal", ("Void", None), token=end))
            if (self.peek("mut") or
                    (self.tokens[self.i].kind == "IDENT" and
                     self.tokens[self.i + 1].text == ":")):
                start = self.take(kind="IDENT")
                mutable = start.text == "mut"
                name = self.take(kind="IDENT").text if mutable else start.text
                self.take(":")
                type_name = self.type_name()
                self.take("=")
                value = self.expr()
                if self.peek(";"):
                    self.take()
                elif not self.peek("}"):
                    self.fail(self.tokens[self.i], "expected line break, semicolon, or closing brace after binding")
                statements.append(Let(name, type_name, value, mutable, start))
                continue
            if self.peek("while"):
                start = self.take()
                condition = self.expr()
                statements.append(While(condition, self.block(), start))
                if self.peek(";"):
                    self.take()
                continue
            if self.peek("for"):
                start = self.take()
                name = self.take(kind="IDENT").text
                self.take("in")
                iterable = self.expr()
                statements.append(For(name, iterable, self.block(), start))
                if self.peek(";"):
                    self.take()
                continue
            if (self.tokens[self.i].kind == "IDENT" and
                    self.tokens[self.i + 1].text == "="):
                name = self.take(kind="IDENT")
                self.take("=")
                value = self.expr()
                if self.peek(";"):
                    self.take()
                elif not self.peek("}"):
                    self.fail(self.tokens[self.i], "expected line break, semicolon, or closing brace after assignment")
                statements.append(Assign(name.text, value, name))
                continue
            candidate = self.expr()
            if self.peek(";"):
                self.take()
                statements.append(candidate)
                if self.peek("}"):
                    end = self.take()
                    return Block(statements, Expr("literal", ("Void", None), token=end))
                continue
            self.take("}")
            return Block(statements, candidate)

    def expr(self, min_prec: int = 0) -> Expr:
        left = self.prefix()
        while True:
            tok = self.tokens[self.i]
            prec = self.PRECEDENCE.get(tok.text, -1)
            if prec < min_prec:
                break
            self.i += 1
            right = self.expr(prec + 1)
            left = Expr("binary", tok.text, (left, right), tok)
        return left

    def prefix(self) -> Expr:
        tok = self.tokens[self.i]
        if tok.text == "@":
            self.i += 1
            name = self.take(kind="IDENT")
            result = Expr("function", name.text, token=tok)
        elif tok.text in {"-", "!"}:
            self.i += 1
            return Expr("unary", tok.text, (self.expr(7),), tok)
        elif tok.text == "match":
            self.i += 1
            subject = self.expr()
            self.take("{")
            arms: list[tuple[Pattern, Expr]] = []
            while not self.peek("}"):
                variant = self.take(kind="IDENT")
                self.take("(")
                bindings: list[str] = []
                if not self.peek(")"):
                    while True:
                        bindings.append(self.take(kind="IDENT").text)
                        if not self.peek(","):
                            break
                        self.take()
                self.take(")")
                self.take("=>")
                body = Expr("block", self.block(), token=variant) if self.peek("{") else self.expr()
                arms.append((Pattern(variant.text, bindings, variant), body))
                if not self.peek(","):
                    break
                self.take()
            self.take("}")
            result = Expr("match", arms, (subject,), tok)
        elif tok.text == "if":
            self.i += 1
            cond = self.expr()
            yes = self.block()
            has_else = self.peek("else")
            if has_else:
                self.take()
                no = self.block()
            else:
                no = Block([], Expr("literal", ("Void", None), token=tok))
            result = Expr("if", has_else, (Expr("block", yes), Expr("block", no), cond), tok)
        elif tok.text == "(":
            self.i += 1
            if self.peek(")"):
                self.fail(tok, "empty parentheses are not a value; Void functions may end without one")
                raise AssertionError
            else:
                result = self.expr()
                self.take(")")
        elif tok.text == "[":
            self.i += 1
            items: list[Expr] = []
            if not self.peek("]"):
                while True:
                    items.append(self.expr())
                    if not self.peek(","):
                        break
                    self.take()
            self.take("]")
            result = Expr("array", None, tuple(items), tok)
        elif tok.kind == "INT":
            self.i += 1
            result = Expr("literal", ("Nat", int(tok.text)), token=tok)
        elif tok.kind == "DEC":
            self.i += 1
            result = Expr("literal", ("Dec", decimal.Decimal(tok.text)), token=tok)
        elif tok.kind == "STRING":
            self.i += 1
            import json
            text = json.loads(tok.text)
            matches = list(re.finditer(r"\$\{([A-Za-z_][A-Za-z0-9_]*)\}", text))
            if matches:
                parts: list[str] = []
                variables: list[Expr] = []
                start = 0
                for match in matches:
                    parts.append(text[start:match.start()])
                    variables.append(Expr("var", match.group(1), token=tok))
                    start = match.end()
                parts.append(text[start:])
                result = Expr("interpolate", tuple(parts), tuple(variables), tok)
            else:
                result = Expr("literal", ("Str", text), token=tok)
        elif tok.text in {"true", "false"}:
            self.i += 1
            result = Expr("literal", ("Bool", tok.text == "true"), token=tok)
        elif tok.kind == "IDENT":
            self.i += 1
            if self.peek("("):
                self.take()
                args: list[Expr] = []
                if not self.peek(")"):
                    while True:
                        args.append(self.expr())
                        if not self.peek(","):
                            break
                        self.take()
                self.take(")")
                result = Expr("call", tok.text, tuple(args), tok)
            else:
                result = Expr("var", tok.text, token=tok)
        else:
            self.fail(tok, f"expected expression, found {tok.text!r}")
            raise AssertionError
        while self.peek("[") or self.peek("."):
            if self.peek("["):
                bracket = self.take()
                index = self.expr()
                self.take("]")
                result = Expr("index", None, (result, index), bracket)
            else:
                dot = self.take()
                field = self.take(kind="IDENT")
                if self.peek("("):
                    self.take()
                    args: list[Expr] = [result]
                    if not self.peek(")"):
                        while True:
                            args.append(self.expr())
                            if not self.peek(","):
                                break
                            self.take()
                    self.take(")")
                    method_names = {
                        "put": "$method_put",
                        "has": "$method_has",
                        "get": "$method_get",
                        "add": "$method_add",
                        "call": "$method_call",
                        "map": "$method_map",
                        "reduce": "$method_reduce",
                    }
                    result = Expr("call", method_names.get(field.text, field.text), tuple(args), field)
                else:
                    result = Expr("field", field.text, (result,), dot)
        return result


PRIMITIVES = {"Nat", "Int", "Dec", "Str", "Bool", "Void", "Range", "Bytes"}
GUARD_BASES = {"Nat", "Int", "Dec", "Str", "Bool"}
INTRINSIC_GENERICS = {"Array": 1, "Map": 2, "Set": 1}


def split_type(name: str) -> tuple[str, list[str]]:
    bracket = name.find("[")
    if bracket < 0 or not name.endswith("]"):
        return name, []
    base = name[:bracket]
    contents = name[bracket + 1:-1]
    arguments: list[str] = []
    start = depth = 0
    for index, character in enumerate(contents):
        if character == "[":
            depth += 1
        elif character == "]":
            depth -= 1
        elif character == "," and depth == 0:
            arguments.append(contents[start:index])
            start = index + 1
    arguments.append(contents[start:])
    return base, arguments


def format_type(base: str, arguments: list[str]) -> str:
    return base if not arguments else f"{base}[{','.join(arguments)}]"


def substitute_type(name: str, substitutions: dict[str, str],
                    parameters: set[str]) -> str:
    if name in parameters:
        return substitutions.get(name, f"${name}")
    base, arguments = split_type(name)
    if not arguments:
        return name
    return format_type(base, [substitute_type(argument, substitutions, parameters)
                              for argument in arguments])


@dataclass
class TypeInfo:
    name: str
    const: Any = None
    has_const: bool = False
    mutable: bool = False


BUILTINS: dict[str, tuple[list[str], str, bool, bool]] = {
    "print": (["Any"], "Void", False, True),
    "read_line": ([], "Str", False, False),
    "read_file": (["Str"], "Str", False, False),
    "write_file": (["Str", "Str"], "Void", False, False),
    "len": (["Str"], "Nat", True, False),
    "append": (["Any", "Any"], "Any", True, False),
    "concat": (["Any", "Any"], "Any", True, False),
    "slice": (["Str", "Nat", "Nat"], "Str", True, False),
    "starts_with": (["Str", "Str"], "Bool", True, False),
    "starts_with_at": (["Str", "Str", "Nat"], "Bool", True, False),
    "reverse": (["Str"], "Str", True, False),
    "is_digit": (["Str"], "Bool", True, False),
    "is_letter": (["Str"], "Bool", True, False),
    "is_whitespace": (["Str"], "Bool", True, False),
    "map": ([], "Any", True, False),
    "map_put": (["Any", "Any", "Any"], "Any", True, False),
    "map_has": (["Any", "Any"], "Bool", True, False),
    "map_get": (["Any", "Any"], "Any", True, False),
    "$method_put": (["Any", "Any", "Any"], "Any", True, False),
    "$method_has": (["Any", "Any"], "Bool", True, False),
    "$method_get": (["Any", "Any"], "Any", True, False),
    "set": ([], "Any", True, False),
    "set_add": (["Any", "Any"], "Any", True, False),
    "set_has": (["Any", "Any"], "Bool", True, False),
    "$method_add": (["Any", "Any"], "Any", True, False),
    "bytes": ([], "Bytes", True, False),
    "byte_append": (["Bytes", "Nat"], "Bytes", True, False),
    "bytes_concat": (["Bytes", "Bytes"], "Bytes", True, False),
    "byte_len": (["Bytes"], "Nat", True, False),
    "byte_get": (["Bytes", "Nat"], "Nat", True, False),
    "utf8_encode": (["Str"], "Bytes", True, False),
    "utf8_decode": (["Bytes"], "Str", True, False),
    "read_bytes": (["Str"], "Bytes", False, False),
    "write_bytes": (["Str", "Bytes"], "Void", False, False),
    "nat_from_str": (["Str"], "Nat", True, False),
    "command_args": ([], "Array[Str]", False, False),
    "environment_has": (["Str"], "Bool", False, False),
    "environment_get": (["Str"], "Str", False, False),
    "eprint": (["Any"], "Void", False, True),
    "process_exit": (["Nat"], "Void", False, False),
    "path_resolve": (["Str"], "Str", False, False),
    "path_parent": (["Str"], "Str", True, False),
    "path_join": (["Str", "Str"], "Str", True, False),
    "path_suffix": (["Str"], "Str", True, False),
    "path_with_suffix": (["Str", "Str"], "Str", True, False),
    "path_is_absolute": (["Str"], "Bool", True, False),
    "file_exists": (["Str"], "Bool", False, False),
    "run_bytecode": (["Bytes"], "Void", False, False),
    "run_bytecode_args": (["Bytes", "Array[Str]"], "Void", False, False),
}


class Checker:
    def __init__(self, program: Program):
        self.program = program
        self.variants: dict[str, tuple[str, list[str], list[str]]] = {}
        for enum in program.enums.values():
            for variant, payload in enum.variants.items():
                if variant in PRIMITIVES or variant in BUILTINS:
                    raise PanackeltyError(f"variant constructor {variant} conflicts with a primitive or built-in")
                if variant in self.variants:
                    raise PanackeltyError(f"variant constructor {variant} is declared by more than one enum")
                self.variants[variant] = (enum.name, enum.type_params, payload)

    def error(self, expr_or_tok: Expr | Token, message: str) -> None:
        tok = expr_or_tok.token if isinstance(expr_or_tok, Expr) else expr_or_tok
        assert tok is not None
        raise PanackeltyError(f"{source_location(tok)}: {message}")

    def base(self, name: str) -> str:
        return self.program.types[name].base if name in self.program.types else name

    def check(self) -> None:
        declared: dict[str, Token] = {}
        for collection in (self.program.types, self.program.records, self.program.enums,
                           self.program.functions):
            for name, item in collection.items():
                if name in declared:
                    self.error(item.token, f"top-level name {name} is already declared")
                if name in BUILTINS or name in self.variants:
                    self.error(item.token, f"top-level name {name} conflicts with a constructor or built-in")
                declared[name] = item.token
        for typedef in self.program.types.values():
            if typedef.base not in GUARD_BASES:
                self.error(typedef.token, f"guarded type base must be a scalar, got {typedef.base}")
            env = {"value": TypeInfo(typedef.base)}
            guard = self.expr(typedef.guard, env, True, {})
            if guard.name != "Bool":
                self.error(typedef.guard, "type guard must produce Bool")
        for record in self.program.records.values():
            if len(set(record.type_params)) != len(record.type_params):
                self.error(record.token, f"duplicate type parameter in record {record.name}")
            seen: set[str] = set()
            for field, type_name in record.fields:
                if field in seen:
                    self.error(record.token, f"duplicate field {field} in record {record.name}")
                seen.add(field)
                self.require_type(type_name, record.token, set(record.type_params))
        for enum in self.program.enums.values():
            if len(set(enum.type_params)) != len(enum.type_params):
                self.error(enum.token, f"duplicate type parameter in enum {enum.name}")
            if not enum.variants:
                self.error(enum.token, f"enum {enum.name} must contain at least one variant")
            for payload in enum.variants.values():
                for type_name in payload:
                    self.require_type(type_name, enum.token, set(enum.type_params))
        for fn in self.program.functions.values():
            for _, type_name in fn.params:
                self.require_type(type_name, fn.token)
            self.require_type(fn.return_type, fn.token, allow_void=True)
        if "main" not in self.program.functions:
            raise PanackeltyError("program has no main function")
        if self.program.functions["main"].params:
            self.error(self.program.functions["main"].token, "main cannot take parameters")
        for fn in self.program.functions.values():
            env = {name: TypeInfo(type_name) for name, type_name in fn.params}
            actual = self.block(fn.body, env, fn.pure, {})
            if not self.assignable(actual, fn.return_type):
                self.error(fn.body.result, f"function {fn.name} returns {actual.name}, expected {fn.return_type}")

    def require_type(self, name: str, tok: Token, type_params: set[str] | None = None,
                     allow_void: bool = False) -> None:
        type_params = type_params or set()
        if name == "Void":
            if allow_void:
                return
            self.error(tok, "Void is only valid as a function return type")
        if name in type_params:
            return
        base, arguments = split_type(name)
        if base in INTRINSIC_GENERICS:
            expected = INTRINSIC_GENERICS[base]
            if len(arguments) != expected:
                self.error(tok, f"{base} expects {expected} type arguments")
            for argument in arguments:
                self.require_type(argument, tok, type_params)
            return
        if base in {"Fn", "PureFn"}:
            if not arguments:
                self.error(tok, f"{base} expects at least a return type")
            for index, argument in enumerate(arguments):
                self.require_type(argument, tok, type_params,
                                  allow_void=index == len(arguments) - 1)
            return
        if base in self.program.records:
            expected = len(self.program.records[base].type_params)
        elif base in self.program.enums:
            expected = len(self.program.enums[base].type_params)
        else:
            expected = 0
        if base in self.program.records or base in self.program.enums:
            if len(arguments) != expected:
                self.error(tok, f"{base} expects {expected} type arguments, got {len(arguments)}")
            for argument in arguments:
                self.require_type(argument, tok, type_params)
            return
        if arguments or (base not in PRIMITIVES and base not in self.program.types):
            self.error(tok, f"unknown type {name}")

    def block(self, block: Block, env: dict[str, TypeInfo], pure: bool,
              facts: dict[str, tuple[int | None, int | None]]) -> TypeInfo:
        local = dict(env)
        for statement in block.statements:
            if isinstance(statement, Let):
                if statement.name in local:
                    self.error(statement.token, f"local {statement.name} shadows an existing binding")
                self.require_type(statement.type_name, statement.token)
                value = self.expr(statement.value, local, pure, facts)
                if not self.assignable_expr(value, statement.type_name, statement.value, facts):
                    self.error(statement.value, f"cannot assign {value.name} to {statement.type_name}; guard is not proven")
                local[statement.name] = TypeInfo(
                    statement.type_name,
                    value.const if not statement.mutable else None,
                    value.has_const and not statement.mutable,
                    statement.mutable,
                )
            elif isinstance(statement, Assign):
                if statement.name not in local:
                    self.error(statement.token, f"unknown name {statement.name}")
                target = local[statement.name]
                if not target.mutable:
                    self.error(statement.token, f"cannot assign to immutable local {statement.name}")
                value = self.expr(statement.value, local, pure, facts)
                if not self.assignable_expr(value, target.name, statement.value, facts):
                    self.error(statement.value, f"cannot assign {value.name} to {target.name}")
            elif isinstance(statement, While):
                condition = self.expr(statement.condition, local, pure, facts)
                if condition.name != "Bool":
                    self.error(statement.condition, "while condition must be Bool")
                body_facts = dict(facts)
                self.add_condition_fact(statement.condition, body_facts, True)
                self.block(statement.body, local, pure, body_facts)
            elif isinstance(statement, For):
                iterable = self.expr(statement.iterable, local, pure, facts)
                if iterable.name == "Range":
                    item_type = "Nat"
                elif iterable.name == "Bytes":
                    item_type = "Nat"
                elif self.is_array(iterable.name):
                    item_type = self.array_element(iterable.name)
                else:
                    self.error(statement.iterable, f"for requires a Range or array, got {iterable.name}")
                if statement.name in local:
                    self.error(statement.token, f"loop variable {statement.name} shadows an existing binding")
                loop_env = dict(local)
                loop_env[statement.name] = TypeInfo(item_type)
                self.block(statement.body, loop_env, pure, facts)
            else:
                self.expr(statement, local, pure, facts)
        return self.expr(block.result, local, pure, facts)

    @staticmethod
    def is_array(name: str) -> bool:
        base, arguments = split_type(name)
        return base == "Array" and len(arguments) == 1

    @staticmethod
    def array_element(name: str) -> str:
        return split_type(name)[1][0]

    def assignable(self, value: TypeInfo, target: str) -> bool:
        if value.name == target:
            return True
        source_generic, source_args = split_type(value.name)
        target_generic, target_args = split_type(target)
        if source_generic in {"Fn", "PureFn"} or target_generic in {"Fn", "PureFn"}:
            return (source_generic == target_generic or
                    (source_generic == "PureFn" and target_generic == "Fn")) and source_args == target_args
        if ((source_generic == target_generic or
             (source_generic == "PureFn" and target_generic == "Fn")) and
                source_args and len(source_args) == len(target_args)):
            return all(
                source.startswith("$") or target_arg.startswith("$") or
                self.assignable(TypeInfo(source), target_arg)
                for source, target_arg in zip(source_args, target_args)
            )
        source_base = self.base(value.name)
        target_base = self.base(target)
        if value.name in self.program.types and target == source_base:
            return True
        if source_base == "Nat" and target_base == "Int" and target not in self.program.types:
            return True
        compatible_bases = source_base == target_base or (source_base == "Nat" and target_base == "Int")
        if target in self.program.types and compatible_bases and value.has_const:
            return bool(self.eval_guard(self.program.types[target].guard, value.const))
        return False

    def infer_type_arguments(self, template: str, actual: str, parameters: set[str],
                             substitutions: dict[str, str]) -> bool:
        if template in parameters:
            current = substitutions.get(template)
            if current is None:
                substitutions[template] = actual
                return True
            merged = self.merge_type_names(current, actual)
            if merged is None:
                return False
            substitutions[template] = merged
            return True
        template_base, template_args = split_type(template)
        actual_base, actual_args = split_type(actual)
        if template_args:
            return (template_base == actual_base and len(template_args) == len(actual_args) and
                    all(self.infer_type_arguments(left, right, parameters, substitutions)
                        for left, right in zip(template_args, actual_args)))
        return self.assignable(TypeInfo(actual), template)

    def merge_type_names(self, left: str, right: str) -> str | None:
        if left == right:
            return left
        if left.startswith("$"):
            return right
        if right.startswith("$"):
            return left
        left_base, left_args = split_type(left)
        right_base, right_args = split_type(right)
        if left_base == right_base and left_args and len(left_args) == len(right_args):
            merged = [self.merge_type_names(a, b) for a, b in zip(left_args, right_args)]
            return format_type(left_base, [item for item in merged if item is not None]) if all(item is not None for item in merged) else None
        if {self.base(left), self.base(right)} == {"Nat", "Int"}:
            return "Int"
        return None

    def assignable_expr(self, value: TypeInfo, target: str, source: Expr,
                        facts: dict[str, tuple[int | None, int | None]]) -> bool:
        if self.assignable(value, target):
            return True
        if target not in self.program.types or source.kind != "var":
            return False
        typedef = self.program.types[target]
        if self.base(value.name) != typedef.base:
            return False
        lower, upper = facts.get(source.value, (None, None))
        if typedef.base == "Nat":
            lower = max(0, lower) if lower is not None else 0
        return self.prove_guard(typedef.guard, lower, upper)

    @staticmethod
    def prove_guard(expr: Expr, lower: int | None, upper: int | None) -> bool:
        if expr.kind == "binary" and expr.value == "&&":
            return all(Checker.prove_guard(item, lower, upper) for item in expr.args)
        if expr.kind == "binary" and expr.value == "||":
            return any(Checker.prove_guard(item, lower, upper) for item in expr.args)
        if (expr.kind != "binary" or expr.args[0].kind != "var" or
                expr.args[0].value != "value" or expr.args[1].kind != "literal"):
            return False
        bound = expr.args[1].value[1]
        if not isinstance(bound, int):
            return False
        if expr.value == ">": return lower is not None and lower > bound
        if expr.value == ">=": return lower is not None and lower >= bound
        if expr.value == "<": return upper is not None and upper < bound
        if expr.value == "<=": return upper is not None and upper <= bound
        if expr.value == "==": return lower == upper == bound
        if expr.value == "!=": return (upper is not None and upper < bound) or (lower is not None and lower > bound)
        return False

    def eval_guard(self, expr: Expr, value: Any) -> Any:
        if expr.kind == "literal":
            return expr.value[1]
        if expr.kind == "var" and expr.value == "value":
            return value
        if expr.kind == "unary":
            item = self.eval_guard(expr.args[0], value)
            return -item if expr.value == "-" else not item
        if expr.kind == "binary":
            a, b = (self.eval_guard(x, value) for x in expr.args)
            return apply_binary(expr.value, a, b)
        raise PanackeltyError("guard contains an unsupported expression")

    def expr(self, expr: Expr, env: dict[str, TypeInfo], pure: bool,
             facts: dict[str, tuple[int | None, int | None]]) -> TypeInfo:
        if expr.kind == "literal":
            return TypeInfo(expr.value[0], expr.value[1], True)
        if expr.kind == "function":
            if expr.value in self.program.functions:
                function = self.program.functions[expr.value]
                arguments = [type_name for _, type_name in function.params] + [function.return_type]
                return TypeInfo(format_type("PureFn" if function.pure else "Fn", arguments))
            self.error(expr, f"unknown function {expr.value}")
        if expr.kind == "var":
            if expr.value not in env:
                self.error(expr, f"unknown name {expr.value}")
            return env[expr.value]
        if expr.kind == "block":
            return self.block(expr.value, env, pure, facts)
        if expr.kind == "interpolate":
            for variable in expr.args:
                value = self.expr(variable, env, pure, facts)
                if self.base(value.name) not in GUARD_BASES:
                    self.error(variable, f"cannot interpolate value of type {value.name}")
            return TypeInfo("Str")
        if expr.kind == "field":
            record_value = self.expr(expr.args[0], env, pure, facts)
            record_name, arguments = split_type(record_value.name)
            if record_name not in self.program.records:
                self.error(expr.args[0], f"field access requires a record, got {record_value.name}")
            record = self.program.records[record_name]
            substitutions = dict(zip(record.type_params, arguments))
            fields = {
                field: substitute_type(type_name, substitutions, set(record.type_params))
                for field, type_name in record.fields
            }
            if expr.value not in fields:
                self.error(expr, f"record {record_name} has no field {expr.value}")
            return TypeInfo(fields[expr.value])
        if expr.kind == "array":
            if not expr.args:
                return TypeInfo("Array[$T]")
            items = [self.expr(item, env, pure, facts) for item in expr.args]
            if any(item.name == "Void" for item in items):
                self.error(expr, "Void expressions cannot be stored in an array")
            element = items[0].name
            for item in items[1:]:
                if self.assignable(item, element):
                    continue
                if self.assignable(items[0], item.name):
                    element = item.name
                    continue
                left_base, right_base = self.base(element), self.base(item.name)
                if {left_base, right_base} == {"Nat", "Int"}:
                    element = "Int"
                    continue
                self.error(expr, f"array elements have incompatible types {element} and {item.name}")
            return TypeInfo(f"Array[{element}]")
        if expr.kind == "index":
            collection = self.expr(expr.args[0], env, pure, facts)
            index = self.expr(expr.args[1], env, pure, facts)
            if not self.is_array(collection.name) and collection.name not in {"Str", "Bytes"}:
                self.error(expr.args[0], f"indexing requires Str, Bytes, or an array, got {collection.name}")
            if self.base(index.name) != "Nat":
                self.error(expr.args[1], f"index must be Nat, got {index.name}")
            if collection.name == "Str":
                return TypeInfo("Str")
            if collection.name == "Bytes":
                return TypeInfo("Nat")
            return TypeInfo(self.array_element(collection.name))
        if expr.kind == "unary":
            item = self.expr(expr.args[0], env, pure, facts)
            if expr.value == "!" and self.base(item.name) == "Bool":
                return TypeInfo("Bool", not item.const, True) if item.has_const else TypeInfo("Bool")
            if expr.value == "-" and self.base(item.name) in {"Nat", "Int", "Dec"}:
                result_type = "Dec" if self.base(item.name) == "Dec" else "Int"
                return TypeInfo(result_type, -item.const, True) if item.has_const else TypeInfo(result_type)
            self.error(expr, f"operator {expr.value} does not accept {item.name}")
        if expr.kind == "binary":
            left = self.expr(expr.args[0], env, pure, facts)
            right = self.expr(expr.args[1], env, pure, facts)
            op = expr.value
            lb, rb = self.base(left.name), self.base(right.name)
            if op == "..":
                if lb != "Nat" or rb != "Nat":
                    self.error(expr, "range bounds must be Nat")
                return TypeInfo("Range")
            if op in {"&&", "||"}:
                if lb != "Bool" or rb != "Bool":
                    self.error(expr, f"operator {op} requires Bool operands")
                return self.fold("Bool", op, left, right)
            if op in {"==", "!="}:
                if lb != rb and {lb, rb} != {"Nat", "Int"}:
                    self.error(expr, f"cannot compare {left.name} with {right.name}")
                return self.fold("Bool", op, left, right)
            if op in {"<", "<=", ">", ">="}:
                if lb not in {"Nat", "Int", "Dec", "Str"} or (lb != rb and {lb, rb} != {"Nat", "Int"}):
                    self.error(expr, f"cannot order {left.name} and {right.name}")
                return self.fold("Bool", op, left, right)
            if op in {"+", "-", "*", "/", "%"}:
                if op == "+" and lb == rb == "Str":
                    return self.fold("Str", op, left, right)
                if lb not in {"Nat", "Int", "Dec"} or (lb != rb and {lb, rb} != {"Nat", "Int"}):
                    self.error(expr, f"operator {op} does not accept {left.name} and {right.name}")
                if "Dec" in {lb, rb} and lb != rb:
                    self.error(expr, "Dec arithmetic requires two Dec operands")
                result = "Dec" if lb == "Dec" else ("Int" if "Int" in {lb, rb} else "Nat")
                folded = self.fold(result, op, left, right)
                if result == "Nat" and op == "-" and not self.nonnegative_sub(expr.args[0], right, left, facts):
                    self.error(expr, "Nat subtraction may underflow; prove the left side is large enough or use Int")
                return folded
        if expr.kind == "call":
            display_name = {
                "$method_put": "put",
                "$method_has": "has",
                "$method_get": "get",
                "$method_add": "add",
                "$method_call": "call",
                "$method_map": "map",
                "$method_reduce": "reduce",
            }.get(expr.value, expr.value)
            if expr.value == "$method_call":
                callable_value = self.expr(expr.args[0], env, pure, facts)
                callable_base, callable_args = split_type(callable_value.name)
                if callable_base not in {"Fn", "PureFn"} or not callable_args:
                    self.error(expr.args[0], f"call expects a callable, got {callable_value.name}")
                if pure and callable_base != "PureFn":
                    self.error(expr, "pure function cannot invoke an impure callable")
                expected = callable_args[:-1]
                actual_args = expr.args[1:]
                if len(actual_args) != len(expected):
                    self.error(expr, f"call expects {len(expected)} arguments, got {len(actual_args)}")
                for index, (argument, target) in enumerate(zip(actual_args, expected), 1):
                    actual = self.expr(argument, env, pure, facts)
                    if not self.assignable_expr(actual, target, argument, facts):
                        self.error(argument, f"argument {index} to call is {actual.name}, expected {target}")
                return TypeInfo(callable_args[-1])
            if expr.value in {"$method_map", "$method_reduce"}:
                collection = self.expr(expr.args[0], env, pure, facts)
                collection_base, collection_args = split_type(collection.name)
                if collection_base != "Array" or len(collection_args) != 1:
                    self.error(expr.args[0], f"{display_name} expects an array, got {collection.name}")
                expected_arity = 2 if expr.value == "$method_map" else 3
                if len(expr.args) != expected_arity:
                    self.error(expr, f"{display_name} expects {expected_arity - 1} arguments, got {len(expr.args) - 1}")
                callback = self.expr(expr.args[-1], env, pure, facts)
                callback_base, callback_args = split_type(callback.name)
                if callback_base != "PureFn":
                    self.error(expr.args[-1], f"{display_name} requires a pure callable, got {callback.name}")
                if expr.value == "$method_map":
                    if len(callback_args) != 2 or not self.assignable(TypeInfo(collection_args[0]), callback_args[0]):
                        self.error(expr.args[-1], f"map callback must accept {collection_args[0]}")
                    if callback_args[-1] == "Void":
                        self.error(expr.args[-1], "map callback cannot return Void")
                    return TypeInfo(format_type("Array", [callback_args[-1]]))
                accumulator = self.expr(expr.args[1], env, pure, facts)
                if (len(callback_args) != 3 or
                        not self.assignable(accumulator, callback_args[0]) or
                        not self.assignable(TypeInfo(collection_args[0]), callback_args[1]) or
                        not self.assignable(TypeInfo(callback_args[2]), accumulator.name)):
                    expected_callback = format_type("PureFn", [accumulator.name, collection_args[0], accumulator.name])
                    self.error(expr.args[-1], f"reduce callback must have type {expected_callback}")
                return TypeInfo(accumulator.name)
            if expr.value in self.program.records:
                record = self.program.records[expr.value]
                params = [type_name for _, type_name in record.fields]
                type_parameters = record.type_params
                result, callee_pure, variadic_any = record.name, True, False
            elif expr.value in self.variants:
                result, type_parameters, params = self.variants[expr.value]
                callee_pure, variadic_any = True, False
            elif expr.value in BUILTINS:
                params, result, callee_pure, variadic_any = BUILTINS[expr.value]
                type_parameters = []
            elif expr.value in self.program.functions:
                fn = self.program.functions[expr.value]
                params, result, callee_pure, variadic_any = [t for _, t in fn.params], fn.return_type, fn.pure, False
                type_parameters = []
            else:
                self.error(expr, f"unknown function {display_name}")
            if pure and not callee_pure:
                self.error(expr, f"pure function cannot call impure function {display_name}")
            if len(expr.args) != len(params):
                self.error(expr, f"{display_name} expects {len(params)} arguments, got {len(expr.args)}")
            if expr.value == "len":
                actual = self.expr(expr.args[0], env, pure, facts)
                if actual.name not in {"Str", "Bytes"} and not self.is_array(actual.name):
                    self.error(expr.args[0], f"len expects Str, Bytes, or array, got {actual.name}")
                return TypeInfo("Nat")
            if expr.value == "append":
                collection = self.expr(expr.args[0], env, pure, facts)
                item = self.expr(expr.args[1], env, pure, facts)
                if item.name == "Void":
                    self.error(expr.args[1], "Void expressions cannot be stored in an array")
                if not self.is_array(collection.name):
                    self.error(expr.args[0], f"append expects an array, got {collection.name}")
                element = self.array_element(collection.name)
                merged = self.merge_type_names(element, item.name)
                if merged is None or (not element.startswith("$") and
                                      not self.assignable(item, element)):
                    self.error(expr.args[1], f"cannot append {item.name} to {collection.name}")
                return TypeInfo(f"Array[{merged}]")
            if expr.value == "concat":
                left = self.expr(expr.args[0], env, pure, facts)
                right = self.expr(expr.args[1], env, pure, facts)
                if not self.is_array(left.name) or not self.is_array(right.name):
                    self.error(expr, "concat expects two arrays")
                merged = self.merge_type_names(self.array_element(left.name),
                                               self.array_element(right.name))
                if merged is None:
                    self.error(expr, f"cannot concatenate {left.name} and {right.name}")
                return TypeInfo(f"Array[{merged}]")
            if expr.value == "map":
                return TypeInfo("Map[$K,$V]")
            if expr.value == "$method_has":
                collection = self.expr(expr.args[0], env, pure, facts)
                base, arguments = split_type(collection.name)
                item = self.expr(expr.args[1], env, pure, facts)
                if base == "Map" and len(arguments) == 2:
                    item_type = self.merge_type_names(arguments[0], item.name)
                    if item_type is None or self.base(item.name) not in GUARD_BASES:
                        self.error(expr.args[1], f"invalid map key type {item.name}")
                elif base == "Set" and len(arguments) == 1:
                    item_type = self.merge_type_names(arguments[0], item.name)
                    if item_type is None or self.base(item.name) not in GUARD_BASES:
                        self.error(expr.args[1], f"invalid set element type {item.name}")
                else:
                    self.error(expr.args[0], f"has expects a Map or Set, got {collection.name}")
                return TypeInfo("Bool")
            if expr.value in {"map_put", "map_has", "map_get", "$method_put", "$method_get"}:
                collection = self.expr(expr.args[0], env, pure, facts)
                base, arguments = split_type(collection.name)
                if base != "Map" or len(arguments) != 2:
                    self.error(expr.args[0], f"{display_name} expects a Map, got {collection.name}")
                key = self.expr(expr.args[1], env, pure, facts)
                key_type = self.merge_type_names(arguments[0], key.name)
                if key_type is None or self.base(key.name) not in GUARD_BASES:
                    self.error(expr.args[1], f"invalid map key type {key.name}")
                if expr.value in {"map_put", "$method_put"}:
                    value = self.expr(expr.args[2], env, pure, facts)
                    if value.name == "Void":
                        self.error(expr.args[2], "Void expressions cannot be stored in a map")
                    value_type = self.merge_type_names(arguments[1], value.name)
                    if value_type is None:
                        self.error(expr.args[2], f"cannot store {value.name} in {collection.name}")
                    return TypeInfo(f"Map[{key_type},{value_type}]")
                return TypeInfo("Bool" if expr.value == "map_has" else arguments[1])
            if expr.value == "set":
                return TypeInfo("Set[$T]")
            if expr.value in {"set_add", "set_has", "$method_add"}:
                collection = self.expr(expr.args[0], env, pure, facts)
                base, arguments = split_type(collection.name)
                if base != "Set" or len(arguments) != 1:
                    self.error(expr.args[0], f"{display_name} expects a Set, got {collection.name}")
                item = self.expr(expr.args[1], env, pure, facts)
                item_type = self.merge_type_names(arguments[0], item.name)
                if item_type is None or self.base(item.name) not in GUARD_BASES:
                    self.error(expr.args[1], f"invalid set element type {item.name}")
                return TypeInfo("Bool" if expr.value == "set_has" else f"Set[{item_type}]")
            substitutions: dict[str, str] = {}
            for i, (arg, expected) in enumerate(zip(expr.args, params), 1):
                actual = self.expr(arg, env, pure, facts)
                if actual.name == "Void":
                    self.error(arg, "Void expression cannot be used as an argument")
                if type_parameters:
                    if not self.infer_type_arguments(expected, actual.name, set(type_parameters), substitutions):
                        self.error(arg, f"argument {i} to {display_name} is {actual.name}, expected {expected}")
                    expected = substitute_type(expected, substitutions, set(type_parameters))
                if expected != "Any" and not expected.startswith("$") and not self.assignable_expr(actual, expected, arg, facts):
                    self.error(arg, f"argument {i} to {display_name} is {actual.name}, expected {expected}")
            if type_parameters:
                result = format_type(result, [substitutions.get(parameter, f"${parameter}")
                                               for parameter in type_parameters])
            return TypeInfo(result)
        if expr.kind == "match":
            subject = self.expr(expr.args[0], env, pure, facts)
            enum_name, enum_arguments = split_type(subject.name)
            if enum_name not in self.program.enums:
                self.error(expr.args[0], f"match requires an enum, got {subject.name}")
            enum = self.program.enums[enum_name]
            substitutions = dict(zip(enum.type_params, enum_arguments))
            seen: set[str] = set()
            result_type: TypeInfo | None = None
            for pattern, body in expr.value:
                if pattern.variant not in enum.variants:
                    self.error(pattern.token, f"{pattern.variant} is not a variant of {enum.name}")
                if pattern.variant in seen:
                    self.error(pattern.token, f"duplicate match arm {pattern.variant}")
                seen.add(pattern.variant)
                payload = [substitute_type(type_name, substitutions, set(enum.type_params))
                           for type_name in enum.variants[pattern.variant]]
                if len(pattern.bindings) != len(payload):
                    self.error(pattern.token, f"pattern {pattern.variant} expects {len(payload)} bindings")
                arm_env = dict(env)
                for binding, type_name in zip(pattern.bindings, payload):
                    if binding in arm_env:
                        self.error(pattern.token, f"pattern binding {binding} shadows an existing binding")
                    arm_env[binding] = TypeInfo(type_name)
                arm_type = self.expr(body, arm_env, pure, facts)
                result_type = arm_type if result_type is None else self.join_types(result_type, arm_type, body)
            missing = set(enum.variants) - seen
            if missing:
                self.error(expr, f"non-exhaustive match; missing {', '.join(sorted(missing))}")
            assert result_type is not None
            return result_type
        if expr.kind == "if":
            cond = self.expr(expr.args[2], env, pure, facts)
            if cond.name != "Bool":
                self.error(expr.args[2], "if condition must be Bool")
            yes_facts, no_facts = dict(facts), dict(facts)
            self.add_condition_fact(expr.args[2], yes_facts, True)
            self.add_condition_fact(expr.args[2], no_facts, False)
            yes = self.block(expr.args[0].value, env, pure, yes_facts)
            if not expr.value:
                return TypeInfo("Void")
            no = self.block(expr.args[1].value, env, pure, no_facts)
            return self.join_types(yes, no, expr)
        self.error(expr, f"unsupported expression {expr.kind}")
        raise AssertionError

    def join_types(self, left: TypeInfo, right: TypeInfo, location: Expr) -> TypeInfo:
        left_generic, left_args = split_type(left.name)
        right_generic, right_args = split_type(right.name)
        if left_generic == right_generic and left_args and len(left_args) == len(right_args):
            merged = self.merge_type_names(left.name, right.name)
            if merged is not None:
                return TypeInfo(merged)
        if self.assignable(left, right.name):
            return TypeInfo(right.name)
        if self.assignable(right, left.name):
            return TypeInfo(left.name)
        left_base, right_base = self.base(left.name), self.base(right.name)
        if left_base == right_base:
            return TypeInfo(left_base)
        if {left_base, right_base} == {"Nat", "Int"}:
            return TypeInfo("Int")
        self.error(location, f"branches have incompatible types {left.name} and {right.name}")
        raise AssertionError

    @staticmethod
    def fold(name: str, op: str, left: TypeInfo, right: TypeInfo) -> TypeInfo:
        if left.has_const and right.has_const:
            try:
                return TypeInfo(name, apply_binary(op, left.const, right.const), True)
            except (ArithmeticError, decimal.DecimalException):
                pass
        return TypeInfo(name)

    @staticmethod
    def add_condition_fact(expr: Expr, facts: dict[str, tuple[int | None, int | None]], truth: bool) -> None:
        if expr.kind != "binary" or expr.args[0].kind != "var" or expr.args[1].kind != "literal":
            return
        name, op, val = expr.args[0].value, expr.value, expr.args[1].value[1]
        if not isinstance(val, int):
            return
        lo, hi = facts.get(name, (None, None))
        if (op, truth) == (">", True) or (op, truth) == ("<=", False): lo = max(lo or val + 1, val + 1)
        if (op, truth) == (">=", True) or (op, truth) == ("<", False): lo = max(lo or val, val)
        if (op, truth) == ("<", True) or (op, truth) == (">=", False): hi = min(hi or val - 1, val - 1)
        if (op, truth) == ("<=", True) or (op, truth) == (">", False): hi = min(hi or val, val)
        if op == "==" and truth: lo = hi = val
        if op == "==" and not truth and val == 0: lo = max(lo or 1, 1)
        facts[name] = (lo, hi)

    @staticmethod
    def nonnegative_sub(left_expr: Expr, right: TypeInfo, left: TypeInfo,
                        facts: dict[str, tuple[int | None, int | None]]) -> bool:
        if left.has_const and right.has_const:
            return left.const >= right.const
        if left_expr.kind == "var" and right.has_const:
            lo, _ = facts.get(left_expr.value, (None, None))
            return lo is not None and lo >= right.const
        return False


def apply_binary(op: str, a: Any, b: Any) -> Any:
    def divide() -> Any:
        if isinstance(a, int) and isinstance(b, int):
            quotient = abs(a) // abs(b)
            return -quotient if (a < 0) != (b < 0) else quotient
        if isinstance(a, decimal.Decimal) and isinstance(b, decimal.Decimal):
            return exact_decimal_divide(a, b)
        return a / b

    if isinstance(a, decimal.Decimal) and isinstance(b, decimal.Decimal) and op in {"+", "-", "*", "%"}:
        a_tuple, b_tuple = a.as_tuple(), b.as_tuple()
        if op == "*":
            precision = len(a_tuple.digits) + len(b_tuple.digits) + 2
        else:
            common_exp = min(a_tuple.exponent, b_tuple.exponent)
            a_width = len(a_tuple.digits) + a_tuple.exponent - common_exp
            b_width = len(b_tuple.digits) + b_tuple.exponent - common_exp
            precision = max(a_width, b_width) + 2
        with decimal.localcontext() as context:
            context.prec = max(precision, 1)
            return {"+": lambda: a + b, "-": lambda: a - b,
                    "*": lambda: a * b, "%": lambda: a % b}[op]()

    return {"+": lambda: a + b, "-": lambda: a - b, "*": lambda: a * b,
            "/": divide, "%": lambda: a % b, "==": lambda: a == b,
            "!=": lambda: a != b, "<": lambda: a < b, "<=": lambda: a <= b,
            ">": lambda: a > b, ">=": lambda: a >= b, "&&": lambda: a and b,
            "||": lambda: a or b}[op]()


def exact_decimal_divide(a: decimal.Decimal, b: decimal.Decimal) -> decimal.Decimal:
    if b == 0:
        raise PanackeltyError("division by zero")
    result = Fraction(a) / Fraction(b)
    denominator = result.denominator
    twos = fives = 0
    while denominator % 2 == 0:
        denominator //= 2
        twos += 1
    while denominator % 5 == 0:
        denominator //= 5
        fives += 1
    if denominator != 1:
        raise PanackeltyError("Dec division has a non-terminating result; explicit rounding is not implemented")
    scale = max(twos, fives)
    coefficient = result.numerator * (2 ** (scale - twos)) * (5 ** (scale - fives))
    sign = 1 if coefficient < 0 else 0
    digits = tuple(int(digit) for digit in str(abs(coefficient)))
    return decimal.Decimal((sign, digits, -scale))


@dataclass
class Code:
    name: str
    params: list[str]
    instructions: list[tuple[str, Any]]
    pure: bool = False


class Compiler:
    def __init__(self, program: Program):
        self.program = program
        self.next_temp = 0
        self.variants = {
            variant: (enum.name, payload)
            for enum in program.enums.values()
            for variant, payload in enum.variants.items()
        }

    def compile(self) -> dict[str, Code]:
        result: dict[str, Code] = {}
        for fn in self.program.functions.values():
            self.next_temp = 0
            code: list[tuple[str, Any]] = []
            self.block(fn.body, code)
            code.append(("RETURN", None))
            result[fn.name] = Code(fn.name, [n for n, _ in fn.params], code, fn.pure)
        return result

    def block(self, block: Block, code: list[tuple[str, Any]]) -> None:
        for statement in block.statements:
            if isinstance(statement, Let):
                self.expr(statement.value, code)
                code.append(("STORE", statement.name))
            elif isinstance(statement, Assign):
                self.expr(statement.value, code)
                code.append(("STORE", statement.name))
            elif isinstance(statement, While):
                loop_start = len(code)
                self.expr(statement.condition, code)
                jump_end = len(code)
                code.append(("JUMP_FALSE", None))
                self.block(statement.body, code)
                code.append(("POP", None))
                code.append(("JUMP", loop_start))
                code[jump_end] = ("JUMP_FALSE", len(code))
            elif isinstance(statement, For):
                iterator = f"$iter{self.next_temp}"
                self.next_temp += 1
                self.expr(statement.iterable, code)
                code.append(("ITER_INIT", iterator))
                loop_start = len(code)
                next_item = len(code)
                code.append(("ITER_NEXT", (iterator, statement.name, None)))
                self.block(statement.body, code)
                code.append(("POP", None))
                code.append(("JUMP", loop_start))
                code[next_item] = ("ITER_NEXT", (iterator, statement.name, len(code)))
            else:
                self.expr(statement, code)
                code.append(("POP", None))
        self.expr(block.result, code)

    def expr(self, expr: Expr, code: list[tuple[str, Any]]) -> None:
        if expr.kind == "literal":
            code.append(("CONST", expr.value))
        elif expr.kind == "function":
            code.append(("CONST", ("Str", expr.value)))
        elif expr.kind == "var":
            code.append(("LOAD", expr.value))
        elif expr.kind == "unary":
            self.expr(expr.args[0], code)
            code.append(("UNARY", expr.value))
        elif expr.kind == "binary":
            if expr.value == "&&":
                self.expr(expr.args[0], code)
                false_jump = len(code)
                code.append(("JUMP_FALSE", None))
                self.expr(expr.args[1], code)
                end_jump = len(code)
                code.append(("JUMP", None))
                code[false_jump] = ("JUMP_FALSE", len(code))
                code.append(("CONST", ("Bool", False)))
                code[end_jump] = ("JUMP", len(code))
            elif expr.value == "||":
                self.expr(expr.args[0], code)
                right_jump = len(code)
                code.append(("JUMP_FALSE", None))
                code.append(("CONST", ("Bool", True)))
                end_jump = len(code)
                code.append(("JUMP", None))
                code[right_jump] = ("JUMP_FALSE", len(code))
                self.expr(expr.args[1], code)
                code[end_jump] = ("JUMP", len(code))
            else:
                self.expr(expr.args[0], code)
                self.expr(expr.args[1], code)
                code.append(("MAKE_RANGE", None) if expr.value == ".." else ("BINARY", expr.value))
        elif expr.kind == "array":
            for item in expr.args:
                self.expr(item, code)
            code.append(("MAKE_ARRAY", len(expr.args)))
        elif expr.kind == "index":
            self.expr(expr.args[0], code)
            self.expr(expr.args[1], code)
            code.append(("INDEX_GET", None))
        elif expr.kind == "field":
            self.expr(expr.args[0], code)
            code.append(("FIELD_GET", expr.value))
        elif expr.kind == "interpolate":
            for variable in expr.args:
                self.expr(variable, code)
            code.append(("INTERPOLATE", expr.value))
        elif expr.kind == "call":
            if expr.value == "$method_call":
                for arg in expr.args:
                    self.expr(arg, code)
                code.append(("CALL_VALUE", len(expr.args) - 1))
                return
            if expr.value in {"$method_map", "$method_reduce"}:
                collection = f"$collection{self.next_temp}"
                callback = f"$callback{self.next_temp}"
                item = f"$item{self.next_temp}"
                iterator = f"$iterator{self.next_temp}"
                accumulator = f"$accumulator{self.next_temp}"
                self.next_temp += 1
                self.expr(expr.args[0], code)
                code.append(("STORE", collection))
                if expr.value == "$method_reduce":
                    self.expr(expr.args[1], code)
                    code.append(("STORE", accumulator))
                else:
                    code.append(("MAKE_ARRAY", 0))
                    code.append(("STORE", accumulator))
                self.expr(expr.args[-1], code)
                code.append(("STORE", callback))
                code.append(("LOAD", collection))
                code.append(("ITER_INIT", iterator))
                loop_start = len(code)
                next_item = len(code)
                code.append(("ITER_NEXT", (iterator, item, None)))
                if expr.value == "$method_map":
                    code.append(("LOAD", accumulator))
                    code.append(("LOAD", callback))
                    code.append(("LOAD", item))
                    code.append(("CALL_VALUE", 1))
                    code.append(("CALL", ("append", 2)))
                    code.append(("STORE", accumulator))
                else:
                    code.append(("LOAD", callback))
                    code.append(("LOAD", accumulator))
                    code.append(("LOAD", item))
                    code.append(("CALL_VALUE", 2))
                    code.append(("STORE", accumulator))
                code.append(("JUMP", loop_start))
                code[next_item] = ("ITER_NEXT", (iterator, item, len(code)))
                code.append(("LOAD", accumulator))
                return
            for arg in expr.args:
                self.expr(arg, code)
            if expr.value in self.program.records:
                fields = [field for field, _ in self.program.records[expr.value].fields]
                code.append(("MAKE_RECORD", (expr.value, fields)))
            elif expr.value in self.variants:
                enum_name, _ = self.variants[expr.value]
                code.append(("MAKE_VARIANT", (enum_name, expr.value, len(expr.args))))
            else:
                code.append(("CALL", (expr.value, len(expr.args))))
        elif expr.kind == "match":
            temporary = f"$match{self.next_temp}"
            self.next_temp += 1
            self.expr(expr.args[0], code)
            code.append(("STORE", temporary))
            end_jumps: list[int] = []
            for pattern, body in expr.value:
                code.append(("LOAD", temporary))
                test = len(code)
                code.append(("MATCH_VARIANT", (pattern.variant, None)))
                for binding in reversed(pattern.bindings):
                    code.append(("STORE", binding))
                self.expr(body, code)
                end_jumps.append(len(code))
                code.append(("JUMP", None))
                code[test] = ("MATCH_VARIANT", (pattern.variant, len(code)))
            code.append(("MATCH_FAIL", None))
            end = len(code)
            for jump in end_jumps:
                code[jump] = ("JUMP", end)
        elif expr.kind == "block":
            self.block(expr.value, code)
        elif expr.kind == "if":
            self.expr(expr.args[2], code)
            jump_false = len(code)
            code.append(("JUMP_FALSE", None))
            self.block(expr.args[0].value, code)
            if expr.value:
                jump_end = len(code)
                code.append(("JUMP", None))
                code[jump_false] = ("JUMP_FALSE", len(code))
                self.block(expr.args[1].value, code)
                code[jump_end] = ("JUMP", len(code))
            else:
                code.append(("POP", None))
                code[jump_false] = ("JUMP_FALSE", len(code))
                code.append(("CONST", ("Void", None)))
        else:
            raise AssertionError(expr.kind)


BYTECODE_MAGIC = b"PANACKBC\x00"
BYTECODE_VERSION = 7
MAX_BYTECODE_BYTES = 16 * 1024 * 1024
MAX_BYTECODE_FUNCTIONS = 4096
MAX_BYTECODE_PARAMETERS = 255
MAX_BYTECODE_INSTRUCTIONS_PER_FUNCTION = 1_000_000
MAX_BYTECODE_TOTAL_INSTRUCTIONS = 4_000_000
MAX_BYTECODE_NAME_BYTES = 1024
MAX_BYTECODE_TEXT_BYTES = 1024 * 1024
MAX_BYTECODE_OPERAND_ITEMS = 65535
MAX_BYTECODE_NUMERIC_DIGITS = 4096
BYTECODE_OPS = (
    "CONST", "LOAD", "STORE", "POP", "UNARY", "BINARY", "MAKE_RANGE",
    "MAKE_ARRAY", "INDEX_GET", "INTERPOLATE", "ITER_INIT", "ITER_NEXT",
    "MAKE_RECORD", "FIELD_GET", "MAKE_VARIANT", "MATCH_VARIANT", "MATCH_FAIL",
    "CALL", "JUMP_FALSE", "JUMP", "RETURN", "CALL_VALUE",
)
VALID_OPS = set(BYTECODE_OPS)
BYTECODE_OPCODE = {op: code for code, op in enumerate(BYTECODE_OPS)}
BYTECODE_UNARY = ("-", "!")
BYTECODE_BINARY = (
    "+", "-", "*", "/", "%", "==", "!=", "<", "<=", ">", ">=", "&&", "||",
)
BYTECODE_CONSTANT_TAGS = ("Nat", "Int", "Dec", "Str", "Bool", "Void")


class _BytecodeWriter:
    def __init__(self) -> None:
        self.data = bytearray()

    def raw(self, value: bytes) -> None:
        if len(self.data) + len(value) > MAX_BYTECODE_BYTES:
            raise PanackeltyError("bytecode artifact exceeds size limit")
        self.data.extend(value)

    def u8(self, value: int) -> None:
        self.raw(value.to_bytes(1, "big"))

    def u16(self, value: int) -> None:
        self.raw(value.to_bytes(2, "big"))

    def u32(self, value: int) -> None:
        self.raw(value.to_bytes(4, "big"))

    def i16(self, value: int) -> None:
        self.raw(value.to_bytes(2, "big", signed=True))

    def name(self, value: str) -> None:
        encoded = value.encode("utf-8")
        self.u16(len(encoded))
        self.raw(encoded)

    def text(self, value: str) -> None:
        encoded = value.encode("utf-8")
        self.u32(len(encoded))
        self.raw(encoded)

    def natural(self, value: int) -> None:
        encoded = b"" if value == 0 else value.to_bytes((value.bit_length() + 7) // 8, "big")
        self.u16(len(encoded))
        self.raw(encoded)

    def integer(self, value: int) -> None:
        self.u8(1 if value < 0 else 0)
        self.natural(abs(value))

    def decimal(self, value: decimal.Decimal) -> None:
        parts = value.as_tuple()
        self.u8(parts.sign)
        self.i16(parts.exponent)
        self.u16(len(parts.digits))
        packed = bytearray()
        for index in range(0, len(parts.digits), 2):
            high = parts.digits[index]
            low = parts.digits[index + 1] if index + 1 < len(parts.digits) else 0xF
            packed.append((high << 4) | low)
        self.raw(bytes(packed))


class _BytecodeReader:
    def __init__(self, data: bytes) -> None:
        self.data = data
        self.offset = 0

    def fail(self, message: str) -> None:
        raise PanackeltyError(f"malformed bytecode payload: {message}")

    def take(self, size: int) -> bytes:
        end = self.offset + size
        if end > len(self.data):
            self.fail("truncated data")
        value = self.data[self.offset:end]
        self.offset = end
        return value

    def u8(self) -> int:
        return int.from_bytes(self.take(1), "big")

    def u16(self) -> int:
        return int.from_bytes(self.take(2), "big")

    def u32(self) -> int:
        return int.from_bytes(self.take(4), "big")

    def i16(self) -> int:
        return int.from_bytes(self.take(2), "big", signed=True)

    def utf8(self, size: int, label: str) -> str:
        try:
            return self.take(size).decode("utf-8")
        except UnicodeDecodeError:
            self.fail(f"invalid UTF-8 in {label}")

    def name(self) -> str:
        size = self.u16()
        if size > MAX_BYTECODE_NAME_BYTES:
            self.fail("name exceeds limit")
        return self.utf8(size, "name")

    def text(self) -> str:
        size = self.u32()
        if size > MAX_BYTECODE_TEXT_BYTES:
            self.fail("text exceeds limit")
        return self.utf8(size, "text")

    def natural(self) -> int:
        size = self.u16()
        max_bytes = (MAX_BYTECODE_NUMERIC_DIGITS * 3322 + 7999) // 8000
        if size > max_bytes:
            self.fail("integer exceeds digit limit")
        encoded = self.take(size)
        if encoded[:1] == b"\x00":
            self.fail("non-minimal integer")
        value = int.from_bytes(encoded, "big")
        if value >= 10 ** MAX_BYTECODE_NUMERIC_DIGITS:
            self.fail("integer exceeds digit limit")
        return value

    def integer(self) -> int:
        sign = self.u8()
        if sign not in {0, 1}:
            self.fail("invalid integer sign")
        value = self.natural()
        if sign and value == 0:
            self.fail("negative zero integer")
        return -value if sign else value

    def decimal(self) -> decimal.Decimal:
        sign = self.u8()
        if sign not in {0, 1}:
            self.fail("invalid decimal sign")
        exponent = self.i16()
        if abs(exponent) > MAX_BYTECODE_NUMERIC_DIGITS:
            self.fail("decimal exponent exceeds limit")
        count = self.u16()
        if count == 0 or count > MAX_BYTECODE_NUMERIC_DIGITS:
            self.fail("decimal coefficient exceeds digit limit")
        encoded = self.take((count + 1) // 2)
        digits: list[int] = []
        for index, byte in enumerate(encoded):
            high, low = byte >> 4, byte & 0xF
            if high > 9:
                self.fail("invalid decimal digit")
            digits.append(high)
            if len(digits) < count:
                if low > 9:
                    self.fail("invalid decimal digit")
                digits.append(low)
            elif low != 0xF:
                self.fail("invalid decimal padding")
        if len(digits) > 1 and digits[0] == 0:
            self.fail("non-minimal decimal coefficient")
        return decimal.Decimal((sign, tuple(digits), exponent))

    def finish(self) -> None:
        if self.offset != len(self.data):
            self.fail("trailing data")


def _encode_instruction(writer: _BytecodeWriter, op: str, arg: Any) -> None:
    writer.u8(BYTECODE_OPCODE[op])
    if op == "CONST":
        type_name, value = arg
        writer.u8(BYTECODE_CONSTANT_TAGS.index(type_name))
        if type_name == "Nat":
            writer.natural(value)
        elif type_name == "Int":
            writer.integer(value)
        elif type_name == "Dec":
            writer.decimal(value)
        elif type_name == "Str":
            writer.text(value)
        elif type_name == "Bool":
            writer.u8(1 if value else 0)
    elif op in {"LOAD", "STORE", "ITER_INIT", "FIELD_GET"}:
        writer.name(arg)
    elif op == "UNARY":
        writer.u8(BYTECODE_UNARY.index(arg))
    elif op == "BINARY":
        writer.u8(BYTECODE_BINARY.index(arg))
    elif op in {"JUMP", "JUMP_FALSE"}:
        writer.u32(arg)
    elif op == "MAKE_ARRAY":
        writer.u16(arg)
    elif op == "INTERPOLATE":
        writer.u16(len(arg))
        for part in arg:
            writer.text(part)
    elif op == "ITER_NEXT":
        writer.name(arg[0])
        writer.name(arg[1])
        writer.u32(arg[2])
    elif op == "MAKE_RECORD":
        writer.name(arg[0])
        writer.u16(len(arg[1]))
        for field in arg[1]:
            writer.name(field)
    elif op == "MAKE_VARIANT":
        writer.name(arg[0])
        writer.name(arg[1])
        writer.u16(arg[2])
    elif op == "MATCH_VARIANT":
        writer.name(arg[0])
        writer.u32(arg[1])
    elif op == "CALL":
        writer.name(arg[0])
        writer.u8(arg[1])
    elif op == "CALL_VALUE":
        writer.u8(arg)


def _decode_instruction(reader: _BytecodeReader, op: str) -> Any:
    if op == "CONST":
        tag = reader.u8()
        if tag >= len(BYTECODE_CONSTANT_TAGS):
            reader.fail(f"unknown constant tag {tag}")
        type_name = BYTECODE_CONSTANT_TAGS[tag]
        if type_name == "Nat":
            value: Any = reader.natural()
        elif type_name == "Int":
            value = reader.integer()
        elif type_name == "Dec":
            value = reader.decimal()
        elif type_name == "Str":
            value = reader.text()
        elif type_name == "Bool":
            value = reader.u8()
            if value not in {0, 1}:
                reader.fail("invalid Bool constant")
            value = bool(value)
        else:
            value = None
        return (type_name, value)
    if op in {"LOAD", "STORE", "ITER_INIT", "FIELD_GET"}:
        return reader.name()
    if op == "UNARY":
        code = reader.u8()
        if code >= len(BYTECODE_UNARY):
            reader.fail(f"unknown unary operator {code}")
        return BYTECODE_UNARY[code]
    if op == "BINARY":
        code = reader.u8()
        if code >= len(BYTECODE_BINARY):
            reader.fail(f"unknown binary operator {code}")
        return BYTECODE_BINARY[code]
    if op in {"JUMP", "JUMP_FALSE"}:
        return reader.u32()
    if op == "MAKE_ARRAY":
        return reader.u16()
    if op == "INTERPOLATE":
        count = reader.u16()
        if count > MAX_BYTECODE_OPERAND_ITEMS:
            reader.fail("interpolation exceeds item limit")
        return tuple(reader.text() for _ in range(count))
    if op == "ITER_NEXT":
        return (reader.name(), reader.name(), reader.u32())
    if op == "MAKE_RECORD":
        type_name = reader.name()
        count = reader.u16()
        if count > MAX_BYTECODE_OPERAND_ITEMS:
            reader.fail("record exceeds item limit")
        return (type_name, tuple(reader.name() for _ in range(count)))
    if op == "MAKE_VARIANT":
        return (reader.name(), reader.name(), reader.u16())
    if op == "MATCH_VARIANT":
        return (reader.name(), reader.u32())
    if op == "CALL":
        return (reader.name(), reader.u8())
    if op == "CALL_VALUE":
        return reader.u8()
    return None


def bytecode_bytes(functions: dict[str, Code]) -> bytes:
    verify_bytecode(functions)
    writer = _BytecodeWriter()
    writer.raw(BYTECODE_MAGIC)
    writer.u16(BYTECODE_VERSION)
    writer.u16(len(functions))
    for fn in (functions[name] for name in sorted(functions)):
        writer.name(fn.name)
        writer.u8(1 if fn.pure else 0)
        writer.u8(len(fn.params))
        for param in fn.params:
            writer.name(param)
        writer.u32(len(fn.instructions))
        for op, arg in fn.instructions:
            _encode_instruction(writer, op, arg)
    return bytes(writer.data)


def write_bytecode(functions: dict[str, Code], path: Path) -> None:
    path.write_bytes(bytecode_bytes(functions))


def decode_bytecode_bytes(data: bytes, label: str = "<memory>") -> dict[str, Code]:
    if len(data) > MAX_BYTECODE_BYTES:
        raise PanackeltyError(f"{label}: bytecode artifact exceeds size limit")
    header_size = len(BYTECODE_MAGIC) + 2
    if len(data) < header_size or data[:len(BYTECODE_MAGIC)] != BYTECODE_MAGIC:
        raise PanackeltyError(f"{label}: not a Panackelty bytecode file")
    version = int.from_bytes(data[len(BYTECODE_MAGIC):header_size], "big")
    if version != BYTECODE_VERSION:
        raise PanackeltyError(f"{label}: unsupported bytecode version {version}")
    try:
        reader = _BytecodeReader(data[header_size:])
        function_count = reader.u16()
        if function_count > MAX_BYTECODE_FUNCTIONS:
            reader.fail("function count exceeds limit")
        functions: dict[str, Code] = {}
        previous_name: str | None = None
        total_instructions = 0
        for _ in range(function_count):
            name = reader.name()
            flags = reader.u8()
            if flags & ~1:
                reader.fail("unknown function flags")
            parameter_count = reader.u8()
            if parameter_count > MAX_BYTECODE_PARAMETERS:
                reader.fail(f"function {name} exceeds parameter limit")
            params = [reader.name() for _ in range(parameter_count)]
            instruction_count = reader.u32()
            if instruction_count > MAX_BYTECODE_INSTRUCTIONS_PER_FUNCTION:
                reader.fail(f"function {name} exceeds instruction limit")
            total_instructions += instruction_count
            if total_instructions > MAX_BYTECODE_TOTAL_INSTRUCTIONS:
                reader.fail("total instruction count exceeds limit")
            if name in functions:
                reader.fail(f"duplicate function {name}")
            if previous_name is not None and name < previous_name:
                reader.fail("functions are not canonically ordered")
            previous_name = name
            instructions: list[tuple[str, Any]] = []
            for _ in range(instruction_count):
                opcode = reader.u8()
                if opcode >= len(BYTECODE_OPS):
                    reader.fail(f"unknown bytecode opcode {opcode}")
                op = BYTECODE_OPS[opcode]
                instructions.append((op, _decode_instruction(reader, op)))
            functions[name] = Code(name, params, instructions, bool(flags & 1))
        reader.finish()
    except PanackeltyError as exc:
        raise PanackeltyError(f"{label}: {exc}") from exc
    verify_bytecode(functions)
    return functions


def load_bytecode(path: Path) -> dict[str, Code]:
    if path.stat().st_size > MAX_BYTECODE_BYTES:
        raise PanackeltyError(f"{path}: bytecode artifact exceeds size limit")
    return decode_bytecode_bytes(path.read_bytes(), str(path))


def verify_bytecode(functions: dict[str, Code]) -> None:
    if len(functions) > MAX_BYTECODE_FUNCTIONS:
        raise PanackeltyError("bytecode exceeds function limit")
    if "main" not in functions:
        raise PanackeltyError("bytecode has no main function")
    if functions["main"].params:
        raise PanackeltyError("bytecode main function cannot take parameters")
    total_instructions = 0
    for table_name, fn in functions.items():
        if len(fn.params) > MAX_BYTECODE_PARAMETERS:
            raise PanackeltyError(f"bytecode function {fn.name} exceeds parameter limit")
        if len(fn.instructions) > MAX_BYTECODE_INSTRUCTIONS_PER_FUNCTION:
            raise PanackeltyError(f"bytecode function {fn.name} exceeds instruction limit")
        total_instructions += len(fn.instructions)
        if total_instructions > MAX_BYTECODE_TOTAL_INSTRUCTIONS:
            raise PanackeltyError("bytecode exceeds total instruction limit")
        if not fn.name or len(set(fn.params)) != len(fn.params):
            raise PanackeltyError(f"invalid function signature for {fn.name!r}")
        if len(fn.name.encode("utf-8")) > MAX_BYTECODE_NAME_BYTES or any(
            len(param.encode("utf-8")) > MAX_BYTECODE_NAME_BYTES for param in fn.params
        ):
            raise PanackeltyError(f"bytecode function {fn.name} exceeds name limit")
        if table_name != fn.name:
            raise PanackeltyError(
                f"function table key {table_name!r} does not match function name {fn.name!r}"
            )
        if not fn.instructions:
            raise PanackeltyError(f"bytecode function {fn.name} is empty")
        has_return = False
        for index, instruction in enumerate(fn.instructions):
            if not isinstance(instruction, tuple) or len(instruction) != 2:
                raise PanackeltyError(f"malformed instruction in {fn.name}")
            op, arg = instruction
            if not isinstance(op, str) or op not in VALID_OPS:
                raise PanackeltyError(f"unknown bytecode instruction {op!r} in {fn.name}")
            has_return = has_return or op == "RETURN"
            if op in {"LOAD", "STORE", "ITER_INIT", "FIELD_GET"} and not isinstance(arg, str):
                raise PanackeltyError(f"invalid {op} operand in {fn.name}")
            if op in {"LOAD", "STORE", "ITER_INIT", "FIELD_GET"} and len(arg.encode("utf-8")) > MAX_BYTECODE_NAME_BYTES:
                raise PanackeltyError(f"{op} operand exceeds name limit in {fn.name}")
            if op in {"POP", "MAKE_RANGE", "INDEX_GET", "MATCH_FAIL", "RETURN"} and arg is not None:
                raise PanackeltyError(f"invalid {op} operand in {fn.name}")
            if op == "UNARY" and arg not in {"-", "!"}:
                raise PanackeltyError(f"invalid UNARY operand in {fn.name}")
            if op == "BINARY" and arg not in {"+", "-", "*", "/", "%", "==", "!=", "<", "<=", ">", ">=", "&&", "||"}:
                raise PanackeltyError(f"invalid BINARY operand in {fn.name}")
            if op in {"JUMP", "JUMP_FALSE"} and (not isinstance(arg, int) or isinstance(arg, bool) or not 0 <= arg < len(fn.instructions)):
                raise PanackeltyError(f"invalid jump target in {fn.name}")
            if op == "MAKE_ARRAY" and (not isinstance(arg, int) or isinstance(arg, bool) or arg < 0):
                raise PanackeltyError(f"invalid MAKE_ARRAY operand in {fn.name}")
            if op == "MAKE_ARRAY" and arg > MAX_BYTECODE_OPERAND_ITEMS:
                raise PanackeltyError(f"MAKE_ARRAY operand exceeds item limit in {fn.name}")
            if op == "MAKE_RECORD":
                if (not isinstance(arg, (list, tuple)) or len(arg) != 2 or
                        not isinstance(arg[0], str) or not isinstance(arg[1], (list, tuple)) or
                        not all(isinstance(field, str) for field in arg[1])):
                    raise PanackeltyError(f"invalid MAKE_RECORD operand in {fn.name}")
                if len(arg[1]) > MAX_BYTECODE_OPERAND_ITEMS:
                    raise PanackeltyError(f"MAKE_RECORD operand exceeds item limit in {fn.name}")
                if len(arg[0].encode("utf-8")) > MAX_BYTECODE_NAME_BYTES or any(
                    len(field.encode("utf-8")) > MAX_BYTECODE_NAME_BYTES for field in arg[1]
                ):
                    raise PanackeltyError(f"MAKE_RECORD operand exceeds name limit in {fn.name}")
            if op == "MAKE_VARIANT":
                if (not isinstance(arg, (list, tuple)) or len(arg) != 3 or
                        not isinstance(arg[0], str) or not isinstance(arg[1], str) or
                        not isinstance(arg[2], int) or isinstance(arg[2], bool) or arg[2] < 0):
                    raise PanackeltyError(f"invalid MAKE_VARIANT operand in {fn.name}")
                if arg[2] > MAX_BYTECODE_OPERAND_ITEMS:
                    raise PanackeltyError(f"MAKE_VARIANT operand exceeds item limit in {fn.name}")
                if len(arg[0].encode("utf-8")) > MAX_BYTECODE_NAME_BYTES or len(arg[1].encode("utf-8")) > MAX_BYTECODE_NAME_BYTES:
                    raise PanackeltyError(f"MAKE_VARIANT operand exceeds name limit in {fn.name}")
            if op == "MATCH_VARIANT":
                if (not isinstance(arg, (list, tuple)) or len(arg) != 2 or
                        not isinstance(arg[0], str) or not isinstance(arg[1], int) or
                        isinstance(arg[1], bool) or not 0 <= arg[1] < len(fn.instructions)):
                    raise PanackeltyError(f"invalid MATCH_VARIANT operand in {fn.name}")
                if len(arg[0].encode("utf-8")) > MAX_BYTECODE_NAME_BYTES:
                    raise PanackeltyError(f"MATCH_VARIANT operand exceeds name limit in {fn.name}")
            if op == "INTERPOLATE" and (not isinstance(arg, (list, tuple)) or not arg or not all(isinstance(part, str) for part in arg)):
                raise PanackeltyError(f"invalid INTERPOLATE operand in {fn.name}")
            if op == "INTERPOLATE" and (
                len(arg) > MAX_BYTECODE_OPERAND_ITEMS or
                any(len(part.encode("utf-8")) > MAX_BYTECODE_TEXT_BYTES for part in arg)
            ):
                raise PanackeltyError(f"INTERPOLATE operand exceeds limit in {fn.name}")
            if op == "CALL":
                if (not isinstance(arg, (list, tuple)) or len(arg) != 2 or not isinstance(arg[0], str) or
                        not isinstance(arg[1], int) or isinstance(arg[1], bool) or arg[1] < 0):
                    raise PanackeltyError(f"invalid CALL operand in {fn.name}")
                callee = arg[0]
                if callee not in functions and callee not in BUILTINS:
                    raise PanackeltyError(f"bytecode calls unknown function {callee}")
                expected_arity = len(BUILTINS[callee][0]) if callee in BUILTINS else len(functions[callee].params)
                if arg[1] != expected_arity:
                    raise PanackeltyError(f"bytecode call to {callee} has invalid arity")
                callee_pure = BUILTINS[callee][2] if callee in BUILTINS else functions[callee].pure
                if fn.pure and not callee_pure:
                    raise PanackeltyError(f"pure bytecode function {fn.name} calls impure function {callee}")
                if len(callee.encode("utf-8")) > MAX_BYTECODE_NAME_BYTES:
                    raise PanackeltyError(f"CALL operand exceeds name limit in {fn.name}")
            if op == "CALL_VALUE" and (not isinstance(arg, int) or isinstance(arg, bool) or not 0 <= arg <= 255):
                raise PanackeltyError(f"invalid CALL_VALUE operand in {fn.name}")
            if op == "ITER_NEXT":
                if (not isinstance(arg, (list, tuple)) or len(arg) != 3 or
                        not isinstance(arg[0], str) or not isinstance(arg[1], str) or
                        not isinstance(arg[2], int) or isinstance(arg[2], bool) or
                        not 0 <= arg[2] < len(fn.instructions)):
                    raise PanackeltyError(f"invalid ITER_NEXT operand in {fn.name}")
                if len(arg[0].encode("utf-8")) > MAX_BYTECODE_NAME_BYTES or len(arg[1].encode("utf-8")) > MAX_BYTECODE_NAME_BYTES:
                    raise PanackeltyError(f"ITER_NEXT operand exceeds name limit in {fn.name}")
            if op == "CONST":
                if not isinstance(arg, (list, tuple)) or len(arg) != 2 or not isinstance(arg[0], str):
                    raise PanackeltyError(f"invalid CONST operand in {fn.name}")
                type_name, value = arg
                valid = (
                    (type_name == "Nat" and isinstance(value, int) and not isinstance(value, bool) and value >= 0) or
                    (type_name == "Int" and isinstance(value, int) and not isinstance(value, bool)) or
                    (type_name == "Dec" and isinstance(value, decimal.Decimal) and value.is_finite()) or
                    (type_name == "Str" and isinstance(value, str)) or
                    (type_name == "Bool" and isinstance(value, bool)) or
                    (type_name == "Void" and value is None)
                )
                if not valid:
                    raise PanackeltyError(f"invalid {type_name} constant in {fn.name}")
                if type_name in {"Nat", "Int"} and len(str(abs(value))) > MAX_BYTECODE_NUMERIC_DIGITS:
                    raise PanackeltyError(f"{type_name} constant exceeds digit limit in {fn.name}")
                if type_name == "Dec" and (
                    len(value.as_tuple().digits) > MAX_BYTECODE_NUMERIC_DIGITS or
                    abs(value.as_tuple().exponent) > MAX_BYTECODE_NUMERIC_DIGITS
                ):
                    raise PanackeltyError(f"Dec constant exceeds digit limit in {fn.name}")
                if type_name == "Str" and len(value.encode("utf-8")) > MAX_BYTECODE_TEXT_BYTES:
                    raise PanackeltyError(f"Str constant exceeds text limit in {fn.name}")
        if not has_return:
            raise PanackeltyError(f"bytecode function {fn.name} has no RETURN")


@dataclass
class Value:
    type_name: str
    data: Any


@dataclass
class Frame:
    function: Code
    locals: dict[str, Value]
    stack: list[Value]
    pc: int = 0


class VM:
    def __init__(self, functions: dict[str, Code], arguments: list[str] | None = None,
                 environment: dict[str, str] | None = None):
        self.functions = functions
        self.arguments = list(arguments or [])
        self.environment = dict(os.environ if environment is None else environment)

    def run(self) -> Value:
        return self.call("main", [])

    @staticmethod
    def pop(frame: Frame) -> Value:
        if not frame.stack:
            raise PanackeltyError("VM trap: operand stack underflow")
        return frame.stack.pop()

    @staticmethod
    def pop_many(frame: Frame, count: int) -> list[Value]:
        if len(frame.stack) < count:
            raise PanackeltyError("VM trap: operand stack underflow")
        if count == 0:
            return []
        values = frame.stack[-count:]
        del frame.stack[-count:]
        return values

    @staticmethod
    def host_path(value: Value) -> Path:
        if "\0" in value.data:
            raise PanackeltyError("VM trap: path contains NUL byte")
        return Path(value.data)

    def call(self, name: str, args: list[Value]) -> Value:
        try:
            return self._execute(name, args)
        except PanackeltyError:
            raise
        except (ArithmeticError, AttributeError, IndexError, KeyError, TypeError, ValueError) as exc:
            raise PanackeltyError("VM trap: invalid bytecode execution") from exc

    def _execute(self, name: str, args: list[Value]) -> Value:
        if name in BUILTINS:
            return self.builtin(name, args)
        entry = self.functions[name]
        frames = [Frame(entry, dict(zip(entry.params, args)), [])]
        while frames:
            frame = frames[-1]
            if frame.pc >= len(frame.function.instructions):
                raise PanackeltyError(f"VM trap: function {frame.function.name} did not return")
            op, arg = frame.function.instructions[frame.pc]
            frame.pc += 1
            if op == "CONST":
                frame.stack.append(Value(arg[0], arg[1]))
            elif op == "LOAD":
                frame.stack.append(frame.locals[arg])
            elif op == "STORE":
                frame.locals[arg] = self.pop(frame)
            elif op == "POP":
                self.pop(frame)
            elif op == "UNARY":
                item = self.pop(frame)
                frame.stack.append(Value("Bool", not item.data) if arg == "!" else Value("Int" if item.type_name != "Dec" else "Dec", -item.data))
            elif op == "BINARY":
                b, a = self.pop(frame), self.pop(frame)
                integer_types = {"Nat", "Int"}
                numeric_pair = (
                    a.type_name in integer_types and b.type_name in integer_types
                ) or a.type_name == b.type_name == "Dec"
                if arg in {"/", "%"} and numeric_pair and b.data == 0:
                    raise PanackeltyError("VM trap: division by zero")
                data = apply_binary(arg, a.data, b.data)
                if arg in {"==", "!=", "<", "<=", ">", ">=", "&&", "||"}:
                    out_type = "Bool"
                elif a.type_name == b.type_name == "Str":
                    out_type = "Str"
                elif "Dec" in {a.type_name, b.type_name}:
                    out_type = "Dec"
                elif "Int" in {a.type_name, b.type_name}:
                    out_type = "Int"
                else:
                    out_type = "Nat"
                if out_type == "Nat" and data < 0:
                    raise PanackeltyError("VM trap: Nat underflow")
                frame.stack.append(Value(out_type, data))
            elif op == "MAKE_RANGE":
                end, start = self.pop(frame), self.pop(frame)
                frame.stack.append(Value("Range", (start.data, end.data)))
            elif op == "MAKE_ARRAY":
                items = self.pop_many(frame, arg)
                element_type = items[0].type_name if items else "Unknown"
                frame.stack.append(Value(f"Array[{element_type}]", list(items)))
            elif op == "MAKE_RECORD":
                type_name, fields = arg
                count = len(fields)
                values = self.pop_many(frame, count)
                frame.stack.append(Value(type_name, dict(zip(fields, values))))
            elif op == "FIELD_GET":
                record = self.pop(frame)
                try:
                    frame.stack.append(record.data[arg])
                except (TypeError, KeyError) as exc:
                    raise PanackeltyError(f"VM trap: {record.type_name} has no field {arg}") from exc
            elif op == "MAKE_VARIANT":
                enum_name, variant, count = arg
                payload = self.pop_many(frame, count)
                frame.stack.append(Value(enum_name, (variant, list(payload))))
            elif op == "MATCH_VARIANT":
                expected, failure = arg
                enum_value = self.pop(frame)
                try:
                    variant, payload = enum_value.data
                except (TypeError, ValueError) as exc:
                    raise PanackeltyError("VM trap: match subject is not an enum") from exc
                if variant != expected:
                    frame.pc = failure
                else:
                    frame.stack.extend(payload)
            elif op == "MATCH_FAIL":
                raise PanackeltyError("VM trap: enum match was not exhaustive")
            elif op == "INDEX_GET":
                index, collection = self.pop(frame), self.pop(frame)
                if index.data < 0 or index.data >= len(collection.data):
                    raise PanackeltyError(
                        f"VM trap: index {index.data} is out of bounds for length {len(collection.data)}"
                    )
                if collection.type_name == "Str":
                    frame.stack.append(Value("Str", collection.data[index.data]))
                elif collection.type_name == "Bytes":
                    frame.stack.append(Value("Nat", collection.data[index.data]))
                else:
                    frame.stack.append(collection.data[index.data])
            elif op == "INTERPOLATE":
                count = len(arg) - 1
                values = self.pop_many(frame, count)
                pieces: list[str] = []
                for i, part in enumerate(arg):
                    pieces.append(part)
                    if i < len(values):
                        pieces.append(self.stringify(values[i]))
                frame.stack.append(Value("Str", "".join(pieces)))
            elif op == "ITER_INIT":
                iterable = self.pop(frame)
                if iterable.type_name == "Range":
                    start, end = iterable.data
                    values = (Value("Nat", item) for item in range(start, end))
                elif iterable.type_name == "Bytes":
                    values = (Value("Nat", item) for item in iterable.data)
                else:
                    values = iter(iterable.data)
                frame.locals[arg] = Value("Iterator", iter(values))
            elif op == "ITER_NEXT":
                iterator, local_name, end = arg
                try:
                    frame.locals[local_name] = next(frame.locals[iterator].data)
                except StopIteration:
                    frame.pc = end
            elif op == "CALL":
                callee, count = arg
                call_args = self.pop_many(frame, count)
                if callee in BUILTINS:
                    frame.stack.append(self.builtin(callee, call_args))
                else:
                    called = self.functions[callee]
                    frames.append(Frame(called, dict(zip(called.params, call_args)), []))
            elif op == "CALL_VALUE":
                call_args = self.pop_many(frame, arg)
                callable_value = self.pop(frame)
                if callable_value.type_name != "Str":
                    raise PanackeltyError("VM trap: indirect call requires a callable")
                callee = callable_value.data
                if callee in BUILTINS:
                    params, _, callee_pure, _ = BUILTINS[callee]
                    if len(params) != arg:
                        raise PanackeltyError("VM trap: indirect call arity mismatch")
                    if frame.function.pure and not callee_pure:
                        raise PanackeltyError("VM trap: pure function invokes impure callable")
                    frame.stack.append(self.builtin(callee, call_args))
                elif callee in self.functions:
                    called = self.functions[callee]
                    if len(called.params) != arg:
                        raise PanackeltyError("VM trap: indirect call arity mismatch")
                    if frame.function.pure and not called.pure:
                        raise PanackeltyError("VM trap: pure function invokes impure callable")
                    frames.append(Frame(called, dict(zip(called.params, call_args)), []))
                else:
                    raise PanackeltyError("VM trap: indirect call target was not found")
            elif op == "JUMP_FALSE":
                if not self.pop(frame).data:
                    frame.pc = arg
            elif op == "JUMP":
                frame.pc = arg
            elif op == "RETURN":
                result = self.pop(frame)
                frames.pop()
                if not frames:
                    return result
                frames[-1].stack.append(result)
        raise AssertionError("unreachable")

    def builtin(self, name: str, args: list[Value]) -> Value:
        if name == "print":
            value = args[0]
            if value.type_name == "Bool":
                print("true" if value.data else "false")
            elif value.type_name == "Void":
                print("void")
            elif value.type_name.startswith("Array["):
                print("[" + ", ".join(VM.display(item) for item in value.data) + "]")
            elif value.type_name.startswith("Map[") or value.type_name.startswith("Set["):
                print(VM.display(value))
            elif value.type_name == "Bytes":
                print(VM.display(value))
            elif isinstance(value.data, (dict, tuple)):
                print(VM.display(value))
            else:
                print(value.data)
            return Value("Void", None)
        if name == "read_line":
            return Value("Str", input())
        if name == "read_file":
            try:
                return Value("Str", self.host_path(args[0]).read_text(encoding="utf-8"))
            except UnicodeDecodeError as exc:
                raise PanackeltyError("I/O error: file is not valid UTF-8") from exc
            except OSError as exc:
                raise PanackeltyError(f"I/O error: {exc}") from exc
        if name == "write_file":
            try:
                self.host_path(args[0]).write_text(args[1].data, encoding="utf-8")
                return Value("Void", None)
            except OSError as exc:
                raise PanackeltyError(f"I/O error: {exc}") from exc
        if name == "read_bytes":
            try:
                return Value("Bytes", self.host_path(args[0]).read_bytes())
            except OSError as exc:
                raise PanackeltyError(f"I/O error: {exc}") from exc
        if name == "write_bytes":
            try:
                self.host_path(args[0]).write_bytes(args[1].data)
                return Value("Void", None)
            except OSError as exc:
                raise PanackeltyError(f"I/O error: {exc}") from exc
        if name == "len":
            return Value("Nat", len(args[0].data))
        if name == "append":
            collection, item = args
            element_type = item.type_name if collection.type_name == "Array[Unknown]" else split_type(collection.type_name)[1][0]
            return Value(f"Array[{element_type}]", list(collection.data) + [item])
        if name == "concat":
            left, right = args
            if left.type_name == "Array[Unknown]":
                type_name = right.type_name
            else:
                type_name = left.type_name
            return Value(type_name, list(left.data) + list(right.data))
        if name == "slice":
            text, start, end = args
            if start.data > end.data or end.data > len(text.data):
                raise PanackeltyError(f"VM trap: invalid string slice {start.data}..{end.data}")
            return Value("Str", text.data[start.data:end.data])
        if name == "starts_with":
            return Value("Bool", args[0].data.startswith(args[1].data))
        if name == "starts_with_at":
            return Value("Bool", args[0].data.startswith(args[1].data, args[2].data))
        if name == "reverse":
            return Value("Str", args[0].data[::-1])
        if name == "is_digit":
            return Value("Bool", len(args[0].data) == 1 and "0" <= args[0].data <= "9")
        if name == "is_letter":
            return Value(
                "Bool",
                len(args[0].data) == 1
                and ("A" <= args[0].data <= "Z" or "a" <= args[0].data <= "z"),
            )
        if name == "is_whitespace":
            return Value("Bool", len(args[0].data) == 1 and args[0].data in " \t\r\n")
        if name == "map":
            return Value("Map[Unknown,Unknown]", [])
        if name in {"map_put", "$method_put"}:
            collection, key, value = args
            entries = [(existing_key, existing_value) for existing_key, existing_value in collection.data
                       if existing_key != key]
            entries.append((key, value))
            return Value(f"Map[{key.type_name},{value.type_name}]", entries)
        if name in {"map_has", "$method_has"} and args[0].type_name.startswith("Map["):
            collection, key = args
            return Value("Bool", any(existing_key == key for existing_key, _ in collection.data))
        if name in {"map_get", "$method_get"}:
            collection, key = args
            for existing_key, value in reversed(collection.data):
                if existing_key == key:
                    return value
            raise PanackeltyError(f"VM trap: map key {VM.display(key)} was not found")
        if name == "set":
            return Value("Set[Unknown]", [])
        if name in {"set_add", "$method_add"}:
            collection, item = args
            values = list(collection.data)
            if item not in values:
                values.append(item)
            return Value(f"Set[{item.type_name}]", values)
        if name in {"set_has", "$method_has"}:
            collection, item = args
            return Value("Bool", item in collection.data)
        if name == "bytes":
            return Value("Bytes", b"")
        if name == "byte_append":
            if args[1].data > 255:
                raise PanackeltyError(f"VM trap: byte value {args[1].data} is outside 0..255")
            return Value("Bytes", args[0].data + bytes([args[1].data]))
        if name == "bytes_concat":
            return Value("Bytes", args[0].data + args[1].data)
        if name == "byte_len":
            return Value("Nat", len(args[0].data))
        if name == "byte_get":
            if args[1].data >= len(args[0].data):
                raise PanackeltyError(f"VM trap: byte index {args[1].data} is out of bounds")
            return Value("Nat", args[0].data[args[1].data])
        if name == "utf8_encode":
            return Value("Bytes", args[0].data.encode("utf-8"))
        if name == "utf8_decode":
            try:
                return Value("Str", args[0].data.decode("utf-8"))
            except UnicodeDecodeError as exc:
                raise PanackeltyError("VM trap: invalid UTF-8") from exc
        if name == "nat_from_str":
            if not args[0].data or not all("0" <= character <= "9" for character in args[0].data):
                raise PanackeltyError(f"VM trap: {args[0].data!r} is not a Nat")
            return Value("Nat", int(args[0].data))
        if name == "command_args":
            return Value("Array[Str]", [Value("Str", item) for item in self.arguments])
        if name == "environment_has":
            return Value("Bool", args[0].data in self.environment)
        if name == "environment_get":
            key = args[0].data
            if key not in self.environment:
                raise PanackeltyError(f"VM trap: environment variable {key!r} was not found")
            return Value("Str", self.environment[key])
        if name == "eprint":
            print(VM.stringify(args[0]), file=sys.stderr)
            return Value("Void", None)
        if name == "process_exit":
            raise PanackeltyProcessExit(args[0].data)
        if name == "path_resolve":
            return Value("Str", str(self.host_path(args[0]).resolve()))
        if name == "path_parent":
            return Value("Str", str(self.host_path(args[0]).parent))
        if name == "path_join":
            left = self.host_path(args[0])
            right = self.host_path(args[1])
            return Value("Str", os.path.normpath(str(left / right)))
        if name == "path_suffix":
            return Value("Str", self.host_path(args[0]).suffix)
        if name == "path_with_suffix":
            path = self.host_path(args[0])
            suffix = self.host_path(args[1])
            return Value("Str", str(path.with_suffix(str(suffix))))
        if name == "path_is_absolute":
            return Value("Bool", self.host_path(args[0]).is_absolute())
        if name == "file_exists":
            return Value("Bool", self.host_path(args[0]).is_file())
        if name == "run_bytecode":
            VM(decode_bytecode_bytes(args[0].data), self.arguments, self.environment).run()
            return Value("Void", None)
        if name == "run_bytecode_args":
            VM(
                decode_bytecode_bytes(args[0].data),
                [item.data for item in args[1].data],
                self.environment,
            ).run()
            return Value("Void", None)
        raise AssertionError(name)

    @staticmethod
    def display(value: Value) -> str:
        if value.type_name == "Str":
            return repr(value.data)
        if value.type_name == "Bool":
            return "true" if value.data else "false"
        if value.type_name.startswith("Array["):
            return "[" + ", ".join(VM.display(item) for item in value.data) + "]"
        if value.type_name.startswith("Map["):
            entries = ", ".join(f"{VM.display(key)}: {VM.display(item)}"
                                for key, item in value.data)
            return "{" + entries + "}"
        if value.type_name.startswith("Set["):
            return "set{" + ", ".join(VM.display(item) for item in value.data) + "}"
        if value.type_name == "Bytes":
            return "bytes(" + value.data.hex() + ")"
        if isinstance(value.data, dict):
            fields = ", ".join(f"{name}: {VM.display(item)}" for name, item in value.data.items())
            return f"{value.type_name}({fields})"
        if (isinstance(value.data, tuple) and len(value.data) == 2 and
                isinstance(value.data[0], str) and isinstance(value.data[1], list)):
            variant, payload = value.data
            return f"{variant}(" + ", ".join(VM.display(item) for item in payload) + ")"
        return str(value.data)

    @staticmethod
    def stringify(value: Value) -> str:
        if value.type_name == "Str":
            return value.data
        if value.type_name == "Bool":
            return "true" if value.data else "false"
        return str(value.data)


def build(path: Path, stdlib_root: Path | None = None) -> dict[str, Code]:
    program = load_source_program(path, stdlib_root)
    Checker(program).check()
    functions = Compiler(program).compile()
    verify_bytecode(functions)
    return functions


def load_source_program(path: Path, stdlib_root: Path | None = None) -> Program:
    combined = Program([], {}, {}, {}, {})
    visiting: list[Path] = []
    visited: set[Path] = set()
    project_root = path.resolve().parent
    library_root = (stdlib_root or DEFAULT_STDLIB_ROOT).resolve()

    def import_path(imported: str, importer: Path) -> Path:
        for namespace, root in (("stdlib", library_root), ("project", project_root)):
            prefix = f"{namespace}/"
            if not imported.startswith(prefix):
                continue
            relative = imported[len(prefix):]
            if relative.endswith(".panack"):
                relative = relative.removesuffix(".panack")
            elif Path(relative).suffix:
                raise PanackeltyError(
                    f"{importer}: logical import extension must be .panack: {imported}"
                )
            parts = relative.split("/")
            if not parts or any(
                not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", part)
                for part in parts
            ):
                raise PanackeltyError(
                    f"{importer}: invalid logical import path: {imported}"
                )
            return root.joinpath(*parts).with_suffix(".panack")

        imported_path = Path(imported)
        if imported_path.is_absolute():
            raise PanackeltyError(f"{importer}: imports must use relative or logical paths")
        if imported_path.suffix != ".panack":
            raise PanackeltyError(f"{importer}: relative import must reference a .panack file")
        return importer.parent / imported_path

    def add_items(target: dict[str, Any], incoming: dict[str, Any], source: Path) -> None:
        for name, item in incoming.items():
            if name in target:
                raise PanackeltyError(f"{source}: duplicate imported declaration {name}")
            target[name] = item

    def visit(source: Path) -> None:
        resolved = source.resolve()
        if resolved in visited:
            return
        if resolved in visiting:
            cycle = " -> ".join(str(item) for item in visiting + [resolved])
            raise PanackeltyError(f"import cycle: {cycle}")
        visiting.append(resolved)
        unit = Parser(lex(resolved.read_text(encoding="utf-8"), str(resolved))).parse()
        for imported in unit.imports:
            visit(import_path(imported, resolved))
        add_items(combined.types, unit.types, resolved)
        add_items(combined.records, unit.records, resolved)
        add_items(combined.enums, unit.enums, resolved)
        add_items(combined.functions, unit.functions, resolved)
        visiting.pop()
        visited.add(resolved)

    visit(path)
    return combined


def load_artifact(path: Path) -> dict[str, Code]:
    if path.suffix == ".bc":
        return load_bytecode(path)
    if path.suffix == ".panack":
        return build(path)
    raise PanackeltyError("expected a .panack source or .bc bytecode file")


def disassemble(functions: dict[str, Code]) -> str:
    lines: list[str] = []
    for fn in functions.values():
        qualifier = "pure " if fn.pure else ""
        lines.append(f"{qualifier}{fn.name}({', '.join(fn.params)})")
        for i, (op, arg) in enumerate(fn.instructions):
            suffix = "" if arg is None else f" {arg!r}"
            lines.append(f"  {i:04d} {op}{suffix}")
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    raw_args = list(sys.argv[1:] if argv is None else argv)
    commands = {"check", "compile", "run", "disasm"}
    if raw_args and raw_args[0] not in commands and not raw_args[0].startswith("-"):
        raw_args.insert(0, "run")

    parser = argparse.ArgumentParser(
        prog="panack",
        usage="panack FILE | panack {check,compile,run,disasm} FILE",
        description="Compile Panackelty source to bytecode and execute it on the Panackelty VM",
    )
    parser.add_argument("command", choices=sorted(commands), help=argparse.SUPPRESS)
    parser.add_argument("file", type=Path)
    parser.add_argument("-o", "--output", type=Path, help="bytecode output path for compile")
    args, program_args = parser.parse_known_args(raw_args)
    try:
        if program_args and args.command != "run":
            raise PanackeltyError("program arguments are only valid with run")
        if args.output is not None and args.command != "compile":
            raise PanackeltyError("--output is only valid with compile")
        if args.command == "compile":
            if args.file.suffix != ".panack":
                raise PanackeltyError("compile expects a .panack source file")
            output = args.output or args.file.with_suffix(".bc")
            if output.resolve() == args.file.resolve():
                raise PanackeltyError("bytecode output cannot overwrite the source file")
            functions = build(args.file)
            write_bytecode(functions, output)
            print(f"wrote {output}")
        elif args.command == "check":
            load_artifact(args.file)
            print("ok")
        elif args.command == "disasm":
            print(disassemble(load_artifact(args.file)))
        else:
            VM(load_artifact(args.file), program_args).run()
        return 0
    except PanackeltyProcessExit as exc:
        return exc.code
    except (PanackeltyError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
