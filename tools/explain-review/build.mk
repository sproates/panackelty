dir := build/explain-review
sources := $(shell find src -name '*.panack')
.PHONY: review
review: $(dir)/explorer.bc $(dir)/terminal
$(dir):
	mkdir -p "$@"
$(dir)/explorer.bc: tools/explain-review/main.panack $(sources) bootstrap/compiler-v9.bc | $(dir)
	@tmp=$$(mktemp "$@.XXXXXX"); trap 'rm -f "$$tmp"' EXIT; \
	  PANACKELTY_BOOTSTRAP_ROOT="$(CURDIR)" ./bin/panack compile "$<" -o "$$tmp" && mv "$$tmp" "$@"
$(dir)/terminal: tools/explain-review/terminal.c | $(dir)
	@tmp=$$(mktemp "$@.XXXXXX"); trap 'rm -f "$$tmp"' EXIT; \
	  $(CC) -std=c11 -O2 -Wall -Wextra -Werror "$<" -o "$$tmp" && chmod +x "$$tmp" && mv "$$tmp" "$@"
.DELETE_ON_ERROR:
