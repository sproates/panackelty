#!/bin/sh
# Verify the checked-in stage-1 seed can execute the namespace path it will
# need to bootstrap the coordinated compiler-source migration.
set -eu

seed=${SEED_COMPILER:-bootstrap/compiler-v9.bc}
stdlib=$(pwd)/src/stdlib
PANACKELTY_NAMESPACE_FIXTURE_VALUE=namespace-ok
export PANACKELTY_NAMESPACE_FIXTURE_VALUE
fixture=tests/fixtures/compiler_contracts/namespaces/emission
workspace=$(mktemp -d "${TMPDIR:-/tmp}/panackelty seed namespace.XXXXXX")
trap 'rm -rf "$workspace"' EXIT HUP INT TERM

fail() {
  echo "namespace seed: $*" >&2
  for log in "$workspace"/*.stderr; do
    if [ -s "$log" ]; then
      echo "--- $log" >&2
      cat "$log" >&2
    fi
  done
  exit 1
}

mkdir "$workspace/project"
cp "$fixture"/*.panack "$workspace/project/"
sed 's/^import "left.panack" as left$/import project\/left as left/; s/^import "right.panack" as right$/import project\/right as right/' \
  "$workspace/project/main.panack" > "$workspace/main.panack"
mv "$workspace/main.panack" "$workspace/project/main.panack"
source="$workspace/project/main.panack"
artifact="$workspace/program.bc"

cat > "$workspace/expected.stdout" <<'EOF'
41
42
43
44
41
42
44
45
51
52
EOF

PANACKELTY_STDLIB_PATH="$stdlib" ./panack-vm run "$seed" check "$source" \
  > "$workspace/check.stdout" 2> "$workspace/check.stderr" || fail "seed rejected namespace source"
printf 'ok\n' > "$workspace/check.expected"
cmp -s "$workspace/check.expected" "$workspace/check.stdout" || fail "unexpected check output"
[ ! -s "$workspace/check.stderr" ] || fail "check wrote diagnostics"

PANACKELTY_STDLIB_PATH="$stdlib" ./panack-vm run "$seed" run "$source" \
  > "$workspace/source.stdout" 2> "$workspace/source.stderr" || fail "seed failed namespace source execution"
cmp -s "$workspace/expected.stdout" "$workspace/source.stdout" || fail "source output mismatch"
[ ! -s "$workspace/source.stderr" ] || fail "source execution wrote diagnostics"

PANACKELTY_STDLIB_PATH="$stdlib" ./panack-vm run "$seed" compile "$source" -o "$artifact" \
  > "$workspace/compile.stdout" 2> "$workspace/compile.stderr" || fail "seed failed namespace compilation"
[ -f "$artifact" ] || fail "seed did not publish bytecode"
[ ! -s "$workspace/compile.stderr" ] || fail "compile wrote diagnostics"
./panack-vm check "$artifact" > "$workspace/bytecode-check.stdout" 2> "$workspace/bytecode-check.stderr" || fail "generated artifact did not verify"
printf 'ok\n' > "$workspace/check.expected"
cmp -s "$workspace/check.expected" "$workspace/bytecode-check.stdout" || fail "unexpected bytecode check output"
[ ! -s "$workspace/bytecode-check.stderr" ] || fail "bytecode check wrote diagnostics"

rm -rf "$workspace/project"
./panack-vm run "$artifact" > "$workspace/bytecode.stdout" 2> "$workspace/bytecode.stderr" || fail "namespace bytecode failed after source removal"
cmp -s "$workspace/expected.stdout" "$workspace/bytecode.stdout" || fail "saved bytecode output mismatch"
[ ! -s "$workspace/bytecode.stderr" ] || fail "bytecode execution wrote diagnostics"

# Exercise an actual compiler leaf module through explicit namespace imports.
# This prevents the coordinated source migration from regressing into the flat
# combined-loader path that previously masked missing exports and imports.
compiler_workspace="$workspace/compiler-source"
mkdir -p "$compiler_workspace/src/compiler"
cp src/compiler/types.panack src/compiler/lexer.panack src/compiler/parser.panack src/compiler/expression_contracts.panack src/compiler/resolver.panack src/compiler/checker.panack src/compiler/module_bindings.panack src/compiler/module_resolution.panack src/compiler/module_signatures.panack src/compiler/purity.panack src/compiler/module_bodies.panack src/compiler/module_effects.panack src/compiler/module_emission.panack src/compiler/emitter.panack "$compiler_workspace/src/compiler/"
cat > "$compiler_workspace/main.panack" <<'EOF'
import stdlib/bytes::{bytes_empty, bytes_push, bytes_join, bytes_length, bytes_at, text_encode_utf8, text_decode_utf8}
import stdlib/option::{option_value_or}
import stdlib/result::{result_value_or}
import stdlib/environment::{environment}
import stdlib/testing::{TestOutcome, TestOutcome.TestPassed, TestOutcome.TestFailed, TestResult, test_expect, test_equal_str, test_equal_nat, test_report}
import stdlib/testing_commands::{TestCommand, test_command_output, test_command_result}
import stdlib/testing_files::{TestWorkspace, test_workspace_create, test_workspace_remove_empty, test_discover_fixtures}
import stdlib/time::{DurationError, DurationError.ZeroDurationDivisor, ClockError, ClockError.ClockUnavailable, duration_seconds, duration_milliseconds, duration_add, duration_subtract, duration_scale, duration_before, duration_as_seconds, duration_divide, duration_ratio}
import stdlib/host::{HostError}
import stdlib/filesystem::{FileKind, FileKind.RegularFile, FileMetadata}
import stdlib/process::{ProcessOutput}
import stdlib/tcp::{TcpServerLimits}
import stdlib/path::{PathError, PathError.EmptyPath}
import project/src/compiler/module_bodies::{check_module_bodies, CheckedModuleBodies, BoundFunctionBody, BodyCheckStatus, BodyCheckStatus.BodyIdentityChecked, BodyCheckStatus.BodyInvalid}
import project/src/compiler/module_effects::{check_module_effects, CheckedModuleEffects}
import project/src/compiler/module_emission::{module_emission_program, ModuleEmissionResult}
import project/src/compiler/emitter::{BytecodeProgram, Instruction, ConstantValue.NatConstant, Instruction.CallInstruction, Instruction.ConstInstruction, Instruction.ReturnInstruction, compile_program}
import project/src/compiler/lexer::{lex}
import project/src/compiler/parser::{parse_module_complete}
import project/src/compiler/expression_contracts::{split_interpolated_string, InterpolatedString, scalar_numeric}
import project/src/compiler/checker::{type_shape, TypeShape}
import project/src/compiler/module_signatures::{signature_effect, SignatureEffect, SignatureEffect.SignaturePure}
import project/src/compiler/purity::{check_program_purity}
import project/src/compiler/module_bindings::{LoadedModule, collect_module_bindings, loaded_module_declaration_index, ModuleIdentity, ModulePackageIdentity.ToolchainModulePackage, ModulePackageIdentity.StandaloneModulePackage}
import project/src/compiler/module_resolution::{binding_graph_module, ModuleBindings, ModuleBindingGraph, ModuleImportEdge}
import project/src/compiler/module_signatures::{check_module_signatures}

record BodyAndEffects {
  bodies: CheckedModuleBodies,
  effects: CheckedModuleEffects
}

pure body_check_result(source: Str): BodyAndEffects {
  body_check_result_at(source, ModuleIdentity(StandaloneModulePackage(), "body-test"))
}

pure body_check_result_at(source: Str, identity: ModuleIdentity): BodyAndEffects {
  match lex(source) {
    Error(errors) => BodyAndEffects(CheckedModuleBodies([], errors), CheckedModuleEffects([], [], map())),
    Ok(tokens) => match parse_module_complete(tokens) {
      Error(error) => BodyAndEffects(CheckedModuleBodies([], [error]), CheckedModuleEffects([], [], map())),
      Ok(parsed) => {
        bindings: ModuleBindings = collect_module_bindings(identity, parsed)
        module: LoadedModule = LoadedModule(identity.path, parsed, parsed.program, bindings, loaded_module_declaration_index(parsed))
        graph: ModuleBindingGraph = ModuleBindingGraph([bindings], [])
        signatures = check_module_signatures(graph, [module], module)
        checked: CheckedModuleBodies = check_module_bodies(graph, [module], signatures.declarations, module)
        effects: CheckedModuleEffects = check_module_effects(
          graph,
          [module],
          signatures.declarations,
          checked.functions,
          true
        )
        BodyAndEffects(
          CheckedModuleBodies(checked.functions, concat(signatures.diagnostics, checked.diagnostics)),
          effects
        )
      }
    }
  }
}

pure path_body_check_result(source: Str): BodyAndEffects {
  body_check_result_at(
    "pub enum PathError { EmptyPath, PathContainsNul, PathNotUtf8, AbsolutePathAppend }\n" + source,
    ModuleIdentity(ToolchainModulePackage(), "path.panack")
  )
}

pure namespace_loaded_module(source: Str, path: Str): Option[LoadedModule] {
  match lex(source) {
    Error(errors) => None(),
    Ok(tokens) => match parse_module_complete(tokens) {
      Error(error) => None(),
      Ok(parsed) => {
        identity: ModuleIdentity = ModuleIdentity(StandaloneModulePackage(), path)
        bindings: ModuleBindings = collect_module_bindings(identity, parsed)
        Some(LoadedModule(path, parsed, parsed.program, bindings, loaded_module_declaration_index(parsed)))
      }
    }
  }
}

pure namespace_identity_checked(bodies: CheckedModuleBodies, name: Str): Bool {
  mut checked: Bool = false
  for body in bodies.functions {
    if body.signature.binding.identity.name == name {
      checked = body.status == BodyIdentityChecked() && len(body.deferred) == 0
    }
  }
  checked
}

pure imported_byte_module_identity_ok(): Bool {
  codec: Option[LoadedModule] = namespace_loaded_module(
    "pub pure encode_u8(value: Nat): Bytes { byte_append(bytes(), value) }",
    "codec.panack"
  )
  application: Option[LoadedModule] = namespace_loaded_module(
    "import project/codec::{encode_u8}\npub pure main(): Bytes { encode_u8(7) }",
    "main.panack"
  )
  match codec {
    None() => false,
    Some(codec_module) => match application {
      None() => false,
      Some(application_module) => {
        modules: [LoadedModule] = [application_module, codec_module]
        graph: ModuleBindingGraph = ModuleBindingGraph(
          [application_module.bindings, codec_module.bindings],
          [ModuleImportEdge(application_module.bindings.identity, 0, codec_module.bindings.identity)]
        )
        codec_signatures = check_module_signatures(graph, modules, codec_module)
        application_signatures = check_module_signatures(graph, modules, application_module)
        signatures = concat(codec_signatures.declarations, application_signatures.declarations)
        codec_bodies: CheckedModuleBodies = check_module_bodies(graph, modules, signatures, codec_module)
        application_bodies: CheckedModuleBodies = check_module_bodies(graph, modules, signatures, application_module)
        emission: ModuleEmissionResult = module_emission_program(
          graph,
          modules,
          signatures,
          concat(application_bodies.functions, codec_bodies.functions),
          application_module.bindings.identity
        )
        emitted_bytecode: BytecodeProgram = compile_program(emission.program)
        imported_call_instructions: [Instruction] = emitted_bytecode.functions[0].instructions
        imported_call_identity_ok: Bool = len(imported_call_instructions) == 3 &&
          imported_call_instructions[0] == ConstInstruction(NatConstant(7)) &&
          imported_call_instructions[1] == CallInstruction("$module1_encode_u8", 1) &&
          imported_call_instructions[2] == ReturnInstruction()
        namespace_identity_checked(codec_bodies, "encode_u8") &&
          namespace_identity_checked(application_bodies, "main") &&
          len(emission.diagnostics) == 0 && len(emitted_bytecode.functions) == 2 &&
          emitted_bytecode.functions[0].name == "main" &&
          emitted_bytecode.functions[1].name == "$module1_encode_u8" && imported_call_identity_ok &&
          len(codec_signatures.diagnostics) == 0 && len(application_signatures.diagnostics) == 0 &&
          len(codec_bodies.diagnostics) == 0 && len(application_bodies.diagnostics) == 0
      }
    }
  }
}

async namespace_tcp_reply(request: Bytes): Result[Bytes,Str] { Ok(request) }

async namespace_tcp_server_signature(): Result[[Result[Unit,Str]],Str] {
  limits = TcpServerLimits(1, 1, 32, 32, 100, 100, 100)
  await tcp_serve("127.0.0.1", 9000, @namespace_tcp_reply, limits)
}

async namespace_tcp_exchange_signature(): Result[Bytes,Str] {
  await tcp_exchange("127.0.0.1", 9000, bytes_empty(), 32, 100)
}

async namespace_async_reader_signature(): Result[Bytes,Str] {
  await async_fake_read(true)
}

main(): Void {
  interpolation: InterpolatedString = split_interpolated_string("\"hello " + "$" + "{name}\"")
  shape: TypeShape = type_shape("Array[Nat]")
  effect: SignatureEffect = signature_effect(true, false)
  quotient_ok: BodyAndEffects = body_check_result("pub pure f(): Nat { quotient(43, 10) }")
  quotient_bad_type: BodyAndEffects = body_check_result("pub pure f(): Nat { quotient(43, \"10\") }")
  quotient_bad_arity: BodyAndEffects = body_check_result("pub pure f(): Nat { quotient(43) }")
  bytes_ok: BodyAndEffects = body_check_result("pub pure f(): Nat { byte_len(bytes_concat(bytes(), byte_append(bytes(), 42))) }")
  byte_parameter_ok: BodyAndEffects = body_check_result("pub pure f(value: Nat): Bytes { byte_append(bytes(), value) }")
  codec_expression_ok: BodyAndEffects = body_check_result("pub pure f(value: Nat): Bytes { byte_append(byte_append(bytes(), quotient(value, 256) % 256), value % 256) }")
  codec_helpers_ok: BodyAndEffects = body_check_result("pure encode_u8(value: Nat): Bytes { byte_append(bytes(), value) } pure encode_u16(value: Nat): Bytes { byte_append(byte_append(bytes(), quotient(value, 256) % 256), value % 256) } pure encode_nat_body(value: Nat): Bytes { if value < 256 { encode_u8(value) } else { byte_append(encode_nat_body(quotient(value, 256)), value % 256) } }")
  utf8_ok: BodyAndEffects = body_check_result("pub pure f(): Str { utf8_decode(utf8_encode(\"ok\")) }")
  byte_get_ok: BodyAndEffects = body_check_result("pub pure f(): Nat { byte_get(bytes(), 0) }")
  bytes_bad_arity: BodyAndEffects = body_check_result("pub pure f(): Bytes { bytes(1) }")
  path_absolute_ok: BodyAndEffects = body_check_result("pub pure f(value: Path): Bool { path_absolute(value) }")
  path_display_ok: BodyAndEffects = body_check_result("pub pure f(value: Path): Str { path_display(value) }")
  path_current_ok: BodyAndEffects = body_check_result("pub pure f(): Path { path_current() }")
  path_native_bytes_ok: BodyAndEffects = body_check_result("pub pure f(value: Path): Bytes { path_native_bytes(value) }")
  path_directory_ok: BodyAndEffects = body_check_result("pub pure f(value: Path): Path { path_directory(value) }")
  path_filename_ok: BodyAndEffects = body_check_result("pub pure f(value: Path): Option[Path] { path_filename(value) }")
  path_from_text_ok: BodyAndEffects = path_body_check_result("pub pure f(): Result[Path, PathError] { path_from_text(\"relative\") }")
  path_from_native_ok: BodyAndEffects = path_body_check_result("pub pure f(): Result[Path, PathError] { path_from_native(bytes()) }")
  path_append_ok: BodyAndEffects = path_body_check_result("pub pure f(value: Path): Result[Path, PathError] { path_append(value, value) }")
  path_to_text_ok: BodyAndEffects = path_body_check_result("pub pure f(value: Path): Result[Str, PathError] { path_to_text(value) }")
  path_from_text_bad_type: BodyAndEffects = path_body_check_result("pub pure f(): Result[Path, PathError] { path_from_text(1) }")
  path_from_native_bad_type: BodyAndEffects = path_body_check_result("pub pure f(): Result[Path, PathError] { path_from_native(\"relative\") }")
  path_append_bad_arity: BodyAndEffects = path_body_check_result("pub pure f(value: Path): Result[Path, PathError] { path_append(value) }")
  path_to_text_bad_type: BodyAndEffects = path_body_check_result("pub pure f(): Result[Str, PathError] { path_to_text(\"relative\") }")
  path_native_bytes_bad_type: BodyAndEffects = body_check_result("pub pure f(): Bytes { path_native_bytes(\"relative\") }")
  path_absolute_bad_type: BodyAndEffects = body_check_result("pub pure f(): Bool { path_absolute(\"relative\") }")
  path_current_bad_arity: BodyAndEffects = body_check_result("pub pure f(): Path { path_current(1) }")
  imported_byte_identity: Bool = imported_byte_module_identity_ok()
  bytes_bad_type: BodyAndEffects = body_check_result("pub pure f(): Bytes { byte_append(bytes(), \"42\") }")
  byte_append_bad_buffer: BodyAndEffects = body_check_result("pub pure f(): Bytes { byte_append(\"42\", 1) }")
  bytes_concat_bad_left: BodyAndEffects = body_check_result("pub pure f(): Bytes { bytes_concat(1, bytes()) }")
  bytes_concat_bad_type: BodyAndEffects = body_check_result("pub pure f(): Bytes { bytes_concat(bytes(), 1) }")
  byte_len_bad_type: BodyAndEffects = body_check_result("pub pure f(): Nat { byte_len(\"42\") }")
  byte_get_bad_bytes: BodyAndEffects = body_check_result("pub pure f(): Nat { byte_get(\"42\", 0) }")
  byte_get_bad_index: BodyAndEffects = body_check_result("pub pure f(): Nat { byte_get(bytes(), \"0\") }")
  utf8_encode_bad_type: BodyAndEffects = body_check_result("pub pure f(): Bytes { utf8_encode(bytes()) }")
  utf8_decode_bad_type: BodyAndEffects = body_check_result("pub pure f(): Str { utf8_decode(\"42\") }")
  impure_effect: BodyAndEffects = body_check_result("pub pure f(): Void { print(\"no\") }")
  bytes_value: Bytes = bytes_join(text_encode_utf8("A"), bytes_push(bytes_empty(), 33))
  stdlib_bytes_ok: Bool = bytes_length(bytes_value) == 2 && bytes_at(bytes_value, 1) == 33 && text_decode_utf8(bytes_value) == "A!"
  stdlib_helpers_ok: Bool = option_value_or[Nat](Some(41), 0) == 41 && result_value_or[Nat,Str](Ok(42), 0) == 42
  stdlib_environment_ok: Bool = match environment("PANACKELTY_NAMESPACE_FIXTURE_VALUE") {
    Some(value) => value == "namespace-ok",
    None() => false
  }
  test_result: TestResult = test_equal_nat("namespace import", 42, 42)
  test_outcome_ok: Bool = test_result.outcome == TestPassed()
  failed_test_result: TestResult = test_expect("expected failure", false, "reason")
  test_failure_shape_ok: Bool = failed_test_result.outcome == TestFailed("reason")
  string_test_result: TestResult = test_equal_str("string assertion", "matched", "matched")
  expectation_result: TestResult = test_expect("expectation", true, "expected success")
  time_sum: Duration = duration_add(duration_seconds(1), duration_milliseconds(1000))
  time_difference: Duration = duration_subtract(time_sum, duration_seconds(1))
  ratio_ok: Bool = match duration_ratio(duration_seconds(2), duration_seconds(1)) {
    Ok(value) => value == 2,
    Error(_) => false
  }
  divide_ok: Bool = match duration_divide(duration_seconds(2), 2) {
    Ok(value) => value == duration_seconds(1),
    Error(_) => false
  }
  divide_error: Bool = match duration_divide(duration_seconds(2), 0) {
    Ok(_) => false,
    Error(_) => true
  }
  ratio_error: Bool = match duration_ratio(duration_seconds(2), duration_seconds(0)) {
    Ok(_) => false,
    Error(_) => true
  }
  zero_divisor_is_reported: Bool = duration_divide(duration_seconds(2), 0) == Error(ZeroDurationDivisor())
  clock_error: ClockError = ClockUnavailable()
  clock_error_is_public: Bool = clock_error == ClockUnavailable()
  stdlib_time_ok: Bool = time_sum == duration_seconds(2) && time_difference == duration_seconds(1) &&
    duration_scale(duration_seconds(1), 2) == duration_seconds(2) &&
    duration_before(duration_seconds(1), duration_seconds(2)) &&
    duration_as_seconds(duration_seconds(2)) == 2 && ratio_ok && divide_ok && divide_error && ratio_error &&
    zero_divisor_is_reported && clock_error_is_public
  host_error: HostError = HostError("read", "not_found", 2)
  file_metadata: FileMetadata = FileMetadata(RegularFile(), 12)
  process_output: ProcessOutput = ProcessOutput(0, 0, bytes_push(bytes_empty(), 65), bytes_empty())
  command_output_result: TestResult = test_command_output("command output", process_output, process_output)
  command_result_result: TestResult = test_command_result("command result", Ok(process_output), process_output)
  tcp_limits: TcpServerLimits = TcpServerLimits(2, 1, 512, 1024, 1000, 1000, 250)
  path_error: PathError = EmptyPath()
  stdlib_types_ok: Bool = host_error.operation == "read" && host_error.code == "not_found" &&
    host_error.native_code == 2 && file_metadata.kind == RegularFile() && file_metadata.size == 12 &&
    process_output.exit_code == 0 && process_output.signal == 0 && bytes_length(process_output.stdout) == 1 &&
    bytes_length(process_output.stderr) == 0 && tcp_limits.clients == 2 && tcp_limits.response_limit == 1024 &&
    path_error == EmptyPath()
  body_contracts_ok: Bool = len(quotient_ok.bodies.diagnostics) == 0 && len(quotient_ok.bodies.functions) == 1 &&
  quotient_ok.bodies.functions[0].status == BodyIdentityChecked() && len(quotient_ok.effects.diagnostics) == 0 &&
  len(quotient_bad_type.bodies.diagnostics) == 1 &&
  len(quotient_bad_type.bodies.functions) == 1 && quotient_bad_type.bodies.functions[0].status == BodyInvalid() &&
  quotient_bad_type.bodies.diagnostics[0].message == "argument 2 to quotient is Str, expected Nat" &&
  len(quotient_bad_arity.bodies.diagnostics) == 1 &&
  quotient_bad_arity.bodies.diagnostics[0].message == "quotient expects 2 arguments, got 1" &&
  len(bytes_ok.bodies.diagnostics) == 0 && bytes_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(byte_parameter_ok.bodies.diagnostics) == 0 && byte_parameter_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(codec_expression_ok.bodies.diagnostics) == 0 && codec_expression_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(codec_helpers_ok.bodies.diagnostics) == 0 && len(codec_helpers_ok.bodies.functions) == 3 &&
  codec_helpers_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  codec_helpers_ok.bodies.functions[1].status == BodyIdentityChecked() &&
  codec_helpers_ok.bodies.functions[2].status == BodyIdentityChecked() &&
  len(utf8_ok.bodies.diagnostics) == 0 && utf8_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(byte_get_ok.bodies.diagnostics) == 0 && byte_get_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(bytes_bad_arity.bodies.diagnostics) == 1 &&
  bytes_bad_arity.bodies.diagnostics[0].message == "bytes expects 0 arguments, got 1" &&
  len(path_absolute_ok.bodies.diagnostics) == 0 && path_absolute_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(path_display_ok.bodies.diagnostics) == 0 && path_display_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(path_current_ok.bodies.diagnostics) == 0 && path_current_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(path_native_bytes_ok.bodies.diagnostics) == 0 && path_native_bytes_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(path_directory_ok.bodies.diagnostics) == 0 && path_directory_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(path_filename_ok.bodies.diagnostics) == 0 && path_filename_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(path_from_text_ok.bodies.diagnostics) == 0 && path_from_text_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(path_from_native_ok.bodies.diagnostics) == 0 && path_from_native_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(path_append_ok.bodies.diagnostics) == 0 && path_append_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(path_to_text_ok.bodies.diagnostics) == 0 && path_to_text_ok.bodies.functions[0].status == BodyIdentityChecked() &&
  len(path_from_text_bad_type.bodies.diagnostics) == 1 &&
  path_from_text_bad_type.bodies.diagnostics[0].message == "argument 1 to path_from_text is Nat, expected Str" &&
  len(path_from_native_bad_type.bodies.diagnostics) == 1 &&
  path_from_native_bad_type.bodies.diagnostics[0].message == "argument 1 to path_from_native is Str, expected Bytes" &&
  len(path_append_bad_arity.bodies.diagnostics) == 1 &&
  path_append_bad_arity.bodies.diagnostics[0].message == "path_append expects 2 arguments, got 1" &&
  len(path_to_text_bad_type.bodies.diagnostics) == 1 &&
  path_to_text_bad_type.bodies.diagnostics[0].message == "argument 1 to path_to_text is Str, expected Path" &&
  len(path_native_bytes_bad_type.bodies.diagnostics) == 1 &&
  path_native_bytes_bad_type.bodies.diagnostics[0].message == "argument 1 to path_native_bytes is Str, expected Path" &&
  len(path_absolute_bad_type.bodies.diagnostics) == 1 &&
  path_absolute_bad_type.bodies.diagnostics[0].message == "argument 1 to path_absolute is Str, expected Path" &&
  len(path_current_bad_arity.bodies.diagnostics) == 1 &&
  path_current_bad_arity.bodies.diagnostics[0].message == "path_current expects 0 arguments, got 1" &&
  len(bytes_bad_type.bodies.diagnostics) == 1 && bytes_bad_type.bodies.functions[0].status == BodyInvalid() &&
  bytes_bad_type.bodies.diagnostics[0].message == "argument 2 to byte_append is Str, expected Nat" &&
  len(byte_append_bad_buffer.bodies.diagnostics) == 1 &&
  byte_append_bad_buffer.bodies.diagnostics[0].message == "argument 1 to byte_append is Str, expected Bytes" &&
  len(bytes_concat_bad_left.bodies.diagnostics) == 1 &&
  bytes_concat_bad_left.bodies.diagnostics[0].message == "argument 1 to bytes_concat is Nat, expected Bytes" &&
  len(bytes_concat_bad_type.bodies.diagnostics) == 1 &&
  bytes_concat_bad_type.bodies.diagnostics[0].message == "argument 2 to bytes_concat is Nat, expected Bytes" &&
  len(byte_len_bad_type.bodies.diagnostics) == 1 &&
  byte_len_bad_type.bodies.diagnostics[0].message == "argument 1 to byte_len is Str, expected Bytes" &&
  len(byte_get_bad_bytes.bodies.diagnostics) == 1 &&
  byte_get_bad_bytes.bodies.diagnostics[0].message == "argument 1 to byte_get is Str, expected Bytes" &&
  len(byte_get_bad_index.bodies.diagnostics) == 1 &&
  byte_get_bad_index.bodies.diagnostics[0].message == "argument 2 to byte_get is Str, expected Nat" &&
  len(utf8_encode_bad_type.bodies.diagnostics) == 1 &&
  utf8_encode_bad_type.bodies.diagnostics[0].message == "argument 1 to utf8_encode is Bytes, expected Str" &&
  len(utf8_decode_bad_type.bodies.diagnostics) == 1 &&
  utf8_decode_bad_type.bodies.diagnostics[0].message == "argument 1 to utf8_decode is Str, expected Bytes" &&
  len(impure_effect.bodies.diagnostics) == 0 && len(impure_effect.effects.diagnostics) == 1 &&
  impure_effect.effects.diagnostics[0].message == "pure function cannot call impure function print"
  match lex("main(): Nat { 42 }") {
    Ok(tokens) => match parse_module_complete(tokens) {
      Ok(parsed) => {
        emitted: BytecodeProgram = compile_program(parsed.program)
        if len(check_program_purity(parsed.program)) != 0 { eprint("purity namespace failed") }
        identity: ModuleIdentity = ModuleIdentity(StandaloneModulePackage(), "test")
        bindings: ModuleBindings = collect_module_bindings(identity, parsed)
        graph: ModuleBindingGraph = ModuleBindingGraph([bindings], [])
        match binding_graph_module(graph, identity) {
          Ok(module) => if body_contracts_ok && imported_byte_identity && stdlib_bytes_ok && stdlib_helpers_ok && stdlib_environment_ok && stdlib_time_ok && stdlib_types_ok && test_outcome_ok && test_failure_shape_ok && len(emitted.functions) == 1 && emitted.functions[0].name == "main" && emitted.functions[0].instructions == [ConstInstruction(NatConstant(42)), ReturnInstruction()] && scalar_numeric("Nat") && interpolation.variables == ["name"] &&
            shape.base == "Array" && shape.arguments == ["Nat"] && effect == SignaturePure() {
            if test_report([test_result, string_test_result, expectation_result, command_output_result, command_result_result]) == 0 {
              print("module graph namespace ok")
            } else {
              eprint("stdlib testing namespace failed")
            }
          } else {
            eprint("expression contracts namespace failed")
          },
          Error(message) => eprint("module graph failed")
        }
      },
      Error(diagnostic) => eprint("parser namespace failed")
    },
    Error(diagnostics) => eprint("lexer namespace failed")
  }
}
EOF
compiler_source="$compiler_workspace/main.panack"
compiler_artifact="$workspace/compiler-source.bc"
printf 'ok\n' > "$workspace/compiler-check.expected"
PANACKELTY_STDLIB_PATH="$stdlib" ./panack-vm run "$seed" check "$compiler_source" \
  > "$workspace/compiler-check.stdout" 2> "$workspace/compiler-check.stderr" || fail "seed rejected compiler namespace source"
cmp -s "$workspace/compiler-check.expected" "$workspace/compiler-check.stdout" || fail "unexpected compiler check output"
[ ! -s "$workspace/compiler-check.stderr" ] || fail "compiler check wrote diagnostics"
PANACKELTY_STDLIB_PATH="$stdlib" ./panack-vm run "$seed" run "$compiler_source" \
  > "$workspace/compiler-source.stdout" 2> "$workspace/compiler-source.stderr" || fail "seed failed compiler namespace source execution"
cat > "$workspace/compiler.expected.stdout" <<'EOF'
PASS namespace import
PASS string assertion
PASS expectation
PASS command output
PASS command result
tests: 5, failures: 0
module graph namespace ok
EOF
cmp -s "$workspace/compiler.expected.stdout" "$workspace/compiler-source.stdout" || fail "compiler namespace output mismatch"
[ ! -s "$workspace/compiler-source.stderr" ] || fail "compiler namespace execution wrote diagnostics"
PANACKELTY_STDLIB_PATH="$stdlib" ./panack-vm run "$seed" compile "$compiler_source" -o "$compiler_artifact" \
  > "$workspace/compiler-compile.stdout" 2> "$workspace/compiler-compile.stderr" || fail "seed failed compiler namespace compilation"
[ -f "$compiler_artifact" ] || fail "seed did not publish compiler namespace bytecode"
[ ! -s "$workspace/compiler-compile.stderr" ] || fail "compiler namespace compile wrote diagnostics"
rm -rf "$compiler_workspace/src"
./panack-vm run "$compiler_artifact" > "$workspace/compiler-bytecode.stdout" 2> "$workspace/compiler-bytecode.stderr" || fail "compiler namespace bytecode failed after source removal"
cmp -s "$workspace/compiler.expected.stdout" "$workspace/compiler-bytecode.stdout" || fail "compiler namespace bytecode output mismatch"
[ ! -s "$workspace/compiler-bytecode.stderr" ] || fail "compiler namespace bytecode execution wrote diagnostics"

echo "namespace seed: source and saved-v9 acceptance passed"
