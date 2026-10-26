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
          "check_module_effects", "loaded_package_binding_graph"};
      for (size_t i = 0; i < sizeof(phases) / sizeof(phases[0]); i++) {
          if (!strcmp(plain, phases[i])) return true;
      }
      return false;
  }
  static void diagnostic_event(const char *event, const char *name,
                               double wall_start, double cpu_start)
  {
      double wall = diagnostic_clock(CLOCK_MONOTONIC);
      double cpu = diagnostic_clock(CLOCK_PROCESS_CPUTIME_ID);
      fprintf(diagnostic_file, "%ld\t%s\t%s\t%.6f\t%.6f\t%.6f\t%.6f\n",
              (long)getpid(), event, name, wall, cpu,
              wall_start ? wall - wall_start : 0,
              cpu_start ? cpu - cpu_start : 0);
      fflush(diagnostic_file);
  }
C
source = replace_once(source, '/* Stack-machine execution.', helper + "\n/* Stack-machine execution.")
source = replace_once(source, '    Function *function;', "    Function *function;\n    bool diagnostic_phase;\n    double diagnostic_wall, diagnostic_cpu;")
source = replace_once(source, '    *frame = (Frame){.function = function};', <<~'C'.rstrip)
    *frame = (Frame){.function = function};
    frame->diagnostic_phase = diagnostic_selected(function->name);
    if (frame->diagnostic_phase) {
        frame->diagnostic_wall = diagnostic_clock(CLOCK_MONOTONIC);
        frame->diagnostic_cpu = diagnostic_clock(CLOCK_PROCESS_CPUTIME_ID);
        diagnostic_event("enter", function->name, 0, 0);
    }
C
source = replace_once(source, '            frame_free(frame);', <<~'C'.rstrip)
            if (frame->diagnostic_phase) {
                diagnostic_event("return", function->name,
                                 frame->diagnostic_wall, frame->diagnostic_cpu);
            }
            frame_free(frame);
C
File.write(path, source)
