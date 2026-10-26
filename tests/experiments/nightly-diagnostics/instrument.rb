# Temporary experiment: instrument the existing seed's execution, without
# rebuilding or replacing the seed. This edit is applied only inside the job.
path = 'src/vm/execute.c'
source = File.read(path)
def replace_once(source, old, replacement)
  abort "instrumentation anchor changed: #{old}" unless source.scan(old).length == 1
  source.sub(old, replacement)
end
helper = <<~'C'
  #include <time.h>
  #include <stdio.h>
  #include <unistd.h>

  /* Inclusive timings, deliberately restricted to coarse compiler phases.
   * No output is sent to the compiler's stdout or stderr. */
  static FILE *diagnostic_file;
  static bool diagnostic_initialized;
  static char diagnostic_last_file[2048];
  static const char *diagnostic_plain(const char *name)
  {
      if (!strncmp(name, "$module", 7)) {
          const char *separator = strchr(name, '_');
          if (separator) return separator + 1;
      }
      return name;
  }
  static Value *diagnostic_field(Value *record, const char *name)
  {
      if (!record || record->kind != V_RECORD) return NULL;
      for (size_t i = 0; i < record->as.named.count; i++) {
          if (!strcmp(record->as.named.names[i], name)) return record->as.named.values[i];
      }
      return NULL;
  }
  static char *diagnostic_text(Value *value)
  {
      if (!value || value->kind != V_STR) return NULL;
      char *copy = malloc(value->as.bytes.length + 1);
      if (!copy) abort();
      for (size_t i = 0; i < value->as.bytes.length; i++) {
          char c = (char)value->as.bytes.data[i];
          copy[i] = (c == '\t' || c == '\n' || c == '\r') ? ' ' : c;
      }
      copy[value->as.bytes.length] = 0;
      return copy;
  }
  static void diagnostic_identity(Function *function, Value **arguments,
                                  char **module, char **body)
  {
      for (size_t i = 0; i < function->param_count; i++) {
          if (!strcmp(function->params[i], "module")) {
              *module = diagnostic_text(diagnostic_field(arguments[i], "source_path"));
          }
          if (!strcmp(function->params[i], "context")) {
              Value *owner = diagnostic_field(arguments[i], "owner");
              *module = diagnostic_text(diagnostic_field(diagnostic_field(owner, "module"), "path"));
              *body = diagnostic_text(diagnostic_field(owner, "name"));
          }
      }
      if (!strcmp(diagnostic_plain(function->name), "lex_file") && function->param_count == 2) {
          *module = diagnostic_text(arguments[1]);
          if (*module) snprintf(diagnostic_last_file, sizeof(diagnostic_last_file), "%s", *module);
      }
      if (!strcmp(diagnostic_plain(function->name), "parse_module_complete")) {
          *module = strdup(diagnostic_last_file);
      }
  }
  static double diagnostic_clock(clockid_t kind)
  {
      struct timespec now;
      if (clock_gettime(kind, &now)) return -1;
      return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
  }
  static bool diagnostic_selected(const char *name)
  {
      if (!diagnostic_initialized) {
          const char *path = getenv("PANACK_DIAGNOSTIC_TRACE");
          if (path && *path) diagnostic_file = fopen(path, "a");
          diagnostic_initialized = true;
      }
      if (!diagnostic_file) return false;
      const char *plain = name;
      if (!strncmp(name, "$module", 7)) {
          plain = strchr(name, '_');
          if (!plain) return false;
          plain++;
      }
      const char *phases[] = {"run_source_command", "prepare_project",
          "load_project", "prepare_loaded_project", "prepare_project_program",
          "module_emission_program", "compile_program", "serialize_bytecode",
          "lex_file", "parse_module_complete", "check_module_signatures",
          "check_module_guards", "check_module_bodies_explained",
          "check_module_effects", "loaded_package_binding_graph", "bind_body_block"};
      for (size_t i = 0; i < sizeof(phases) / sizeof(phases[0]); i++) {
          if (!strcmp(plain, phases[i])) return true;
      }
      return false;
  }
  static void diagnostic_event(const char *event, const char *name,
                               double wall_start, double cpu_start,
                               const char *module, const char *body)
  {
      double wall = diagnostic_clock(CLOCK_MONOTONIC);
      double cpu = diagnostic_clock(CLOCK_PROCESS_CPUTIME_ID);
      fprintf(diagnostic_file, "%ld\t%s\t%s\t%.6f\t%.6f\t%.6f\t%.6f\t%s\t%s\n",
              (long)getpid(), event, name, wall, cpu,
              wall_start ? wall - wall_start : 0,
              cpu_start ? cpu - cpu_start : 0, module ? module : "", body ? body : "");
      fflush(diagnostic_file);
  }
  static void diagnostic_stall(const char *name, const char *module, const char *body)
  {
      const char *phase = getenv("PANACK_DIAGNOSTIC_STALL");
      const char *wanted_module = getenv("PANACK_DIAGNOSTIC_STALL_MODULE");
      const char *wanted_body = getenv("PANACK_DIAGNOSTIC_STALL_FUNCTION");
      if (phase && !strcmp(phase, diagnostic_plain(name)) &&
          (!wanted_module || (module && strstr(module, wanted_module))) &&
          (!wanted_body || (body && !strcmp(body, wanted_body)))) {
          /* Deliberately ignores the VM's cooperative stop flag: the parent
           * must also prove its forced process-group cleanup works. */
          for (;;) pause();
      }
  }
C
source = replace_once(source, '/* Stack-machine execution.', helper + "\n/* Stack-machine execution.")
source = replace_once(source, '    Function *function;', "    Function *function;\n    bool diagnostic_phase;\n    double diagnostic_wall, diagnostic_cpu;\n    char *diagnostic_module, *diagnostic_body;")
source = replace_once(source, "    free(frame->stack);", "    free(frame->diagnostic_module);\n    free(frame->diagnostic_body);\n    free(frame->stack);")
source = replace_once(source, '    *frame = (Frame){.function = function};', <<~'C'.rstrip)
    *frame = (Frame){.function = function};
    frame->diagnostic_phase = diagnostic_selected(function->name);
    /* Nested blocks have the same owner. Observe only the function's root
     * block, avoiding floods of expression-level events. */
    if (frame->diagnostic_phase && !strcmp(diagnostic_plain(function->name), "bind_body_block")) {
        frame->diagnostic_phase = function->param_count == 5 &&
            arguments[4]->kind == V_ARRAY && arguments[4]->as.sequence.count == 1;
    }
    if (frame->diagnostic_phase) {
        diagnostic_identity(function, arguments, &frame->diagnostic_module, &frame->diagnostic_body);
        frame->diagnostic_wall = diagnostic_clock(CLOCK_MONOTONIC);
        frame->diagnostic_cpu = diagnostic_clock(CLOCK_PROCESS_CPUTIME_ID);
        diagnostic_event("enter", function->name, 0, 0, frame->diagnostic_module, frame->diagnostic_body);
        diagnostic_stall(function->name, frame->diagnostic_module, frame->diagnostic_body);
    }
C
source = replace_once(source, '            frame_free(frame);', <<~'C'.rstrip)
            if (frame->diagnostic_phase) {
                diagnostic_event("return", function->name,
                                 frame->diagnostic_wall, frame->diagnostic_cpu,
                                 frame->diagnostic_module, frame->diagnostic_body);
            }
            frame_free(frame);
C
File.write(path, source)
