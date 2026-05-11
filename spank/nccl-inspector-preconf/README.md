# NCCL Inspector Pre-Configuration plugin

<p align="center">
<img alt="Logo of the nccl-inspector-preconf plugin depicting a leprechaun" src="./assets/snccliprecon.png" width="400" height="auto"/>
</p>

`nccl-inspector-preconf` is a Slurm SPANK plugin that
prepares [NCCL Inspector](https://docs.nvidia.com/networking/display/hpcxv2251/spectrum-x-nccl-plugin) environment
variables before user code starts.

> [!NOTE]
> Short name is `snccliprecon` (read as `Sneaky Leprechaun`)

## What it does

The plugin configures NCCL Inspector consistently on worker nodes by:

- enabling or disabling the plugin per job
- setting `NCCL_PROFILER_PLUGIN`
- setting `NCCL_INSPECTOR_DUMP_DIR`
- setting `NCCL_INSPECTOR_PROM_DUMP`
- setting `NCCL_INSPECTOR_DUMP_VERBOSE`
- setting `NCCL_INSPECTOR_DUMP_THREAD_INTERVAL_MICROSECONDS`
- expanding Slurm placeholders in the dump directory
- creating the dump directory with permissive permissions
- creating an Enroot mount file so the profiler plugin and dump directory are visible inside containers

The plugin also guards selected hooks so they run only once per worker host for a given job step.

## Configuration order

Settings are applied in this order:

1. Built-in defaults
2. Arguments from `plugstack.conf`
3. Environment variables
4. `srun` plugin options

To inspect SPANK options:

```bash
srun --help
```

Search for `Options provided by plugins`.

## Supported arguments

The plugin currently supports:

- `enabled`
- `profiler-plugin`
- `dump-dir`
- `prom-dump`
- `dump-verbose`
- `dump-thread-interval-microseconds`

Each argument also has a matching environment variable rendered from the
uppercased plugin prefix, for example:

- `SNCCLIPRECON_ENABLED`
- `SNCCLIPRECON_PROFILER_PLUGIN`
- `SNCCLIPRECON_DUMP_DIR`
- `SNCCLIPRECON_PROM_DUMP`
- `SNCCLIPRECON_DUMP_VERBOSE`
- `SNCCLIPRECON_DUMP_THREAD_INTERVAL_MICROSECONDS`

## Dump directory placeholders

`dump-dir` supports Slurm-style substitutions:

- `%j` for job ID
- `%s` for step ID
- `%J` for `<job ID>.<step ID>`

The plugin first applies the job-level expansion and later, on the worker,
applies the step-level expansion before ensuring the final directory exists.

## Enroot integration

If `/etc/enroot/mounts.d` exists on the worker, the plugin creates a mount file
for the current job step so that:

- the NCCL profiler plugin shared object is available inside the container
- the final dump directory is available inside the container

The mount file is removed during task exit.

## Behavior notes

- The plugin acts only in remote SPANK contexts for runtime setup.
- Batch-script sentinel step IDs are ignored for once-per-worker hook guards.
- Per-worker durable lock files are stored under `/tmp/nccl_inspector_preconf`.

## Development

### Fetch Slurm headers

```bash
make docker-headers
```

This populates `src/vendor/slurm`.

You can override the Slurm version:

```bash
make SLURM_VERSION=<VERSION> docker-headers
```

### Build the plugin

```bash
make docker-build
```

Useful overrides:

```bash
make ARCH=amd64 docker-build
make ARCH=arm64 docker-build
make SLURM_VERSION=<VERSION> docker-build
```

The resulting shared object is written to:

```text
build/spank_nccl_inspector_preconf.so
```

### Redeploy to a cluster

```bash
make redeploy
```

You can adjust pod counts with:

```bash
make NODE_COUNT_WORKER=8 NODE_COUNT_LOGIN=3 redeploy
```
