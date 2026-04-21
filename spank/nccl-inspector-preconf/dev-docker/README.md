# Docker Helpers

This directory exists to support local development of the `nccl-inspector-preconf` SPANK plugin.

The plugin is built with `cgo` and depends on Slurm headers and a Slurm-compatible build environment.
Those inputs need to match the Slurm image variant the plugin will run with, so these Dockerfiles provide a reproducible
way to work against the same base image instead of relying on whatever happens to be installed on the host.

These Dockerfiles are development helpers.
They are not part of the runtime artifact and are not used by CI.

- `headers.dockerfile` exists to copy the Slurm headers out of the target Slurm image

  Use it when you need the exact `/usr/include/slurm` tree from a specific image version.

- `builder.dockerfile` exists to build the plugin inside a matching Slurm container image

  It installs Go, copies `src/`, and produces `build/${PLUGIN_NAME}.so`.
  
  It supports both `release` and `debug` builds through the `MODE` build arg.
