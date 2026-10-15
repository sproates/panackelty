#!/usr/bin/env sh
set -eu

ruby <<'RUBY'
require 'psych'

paths = Dir.glob('.github/workflows/*.{yml,yaml}').sort
abort 'no GitHub Actions workflow files found' if paths.empty?

paths.each do |path|
  document = Psych.parse_file(path)
  root = document&.root
  unless root.is_a?(Psych::Nodes::Mapping)
    abort "#{path}: workflow must be a YAML mapping"
  end

  entries = root.children.each_slice(2).to_h do |key, value|
    [key.value, value]
  end
  %w[name on jobs].each do |key|
    abort "#{path}: missing top-level #{key}" unless entries.key?(key)
  end

  jobs = entries.fetch('jobs')
  unless jobs.is_a?(Psych::Nodes::Mapping) && !jobs.children.empty?
    abort "#{path}: jobs must be a non-empty mapping"
  end
  jobs.children.each_slice(2) do |name, definition|
    unless definition.is_a?(Psych::Nodes::Mapping)
      abort "#{path}: job #{name.value} must be a mapping"
    end
  end
end

puts "Validated YAML syntax and top-level workflow structure for #{paths.length} workflows."
RUBY
