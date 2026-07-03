# Sigward

Kernel-enforced execution integrity for cloud-native containers.

Sigward makes the kernel refuse to run anything you didn't sign. Using
[BPF-LSM](https://docs.ebpf.io/linux/program-type/BPF_PROG_TYPE_LSM/) and the kernel's IMA
file-hashing, Sigward verifies — at execution time, in the kernel — that every binary and
shared object a container runs matches a cryptographically signed SBOM attached to its image.
Anything unattested or tampered with is blocked before it executes.

Sigward is built on [Inspektor Gadget](https://github.com/inspektor-gadget/inspektor-gadget)
and complements [Micromize](https://github.com/micromize-dev/micromize).

## The idea

Supply-chain tooling proves what's *in* your image — SBOMs, signatures, attestations. Sigward
carries that proof all the way to the syscall: the same signed SBOM becomes a kernel-enforced
execution allowlist. If a file wasn't declared and signed in the image's SBOM, or its on-disk
hash doesn't match, it never runs.

- **Signed, not assumed** — the allowlist is your image's SBOM, delivered as a cosign
  attestation (DSSE / in-toto SPDX).
- **Kernel-enforced** — decisions happen at `bprm_check_security` and `mmap_file` using
  `bpf_ima_file_hash`. There is no userspace path to bypass.
- **Runtime, not admission** — verification is per-exec at runtime, not a one-time check at
  image pull or admission.

## What Sigward does

For each container it manages, Sigward fetches the image's signed SPDX SBOM, extracts the
declared file → SHA256 map, and installs it as a per-mount-namespace allowlist in the kernel.
At execution time it:

- **Blocks unattested files** — a binary or shared object whose path is not in the image's SBOM.
- **Blocks tampered files** — a file whose on-disk IMA hash does not match the signed SBOM.

Enforcement happens before the code runs. In audit mode (`--enforce=false`) violations are only
logged.

## Quickstart

### Docker

```bash
docker run -it \
  --name sigward \
  --pid=host \
  --privileged \
  -v /sys/fs/bpf:/sys/fs/bpf \
  -v /sys/kernel/debug:/sys/kernel/debug \
  -v /sys/kernel/security:/sys/kernel/security:ro \
  -v /bin:/host/bin \
  -v /proc:/host/proc \
  -v /run:/host/run \
  -v /usr:/host/usr \
  ghcr.io/micromize-dev/sigward:latest
```

For images in private registries, mount a Docker config so Sigward can pull the SBOM
attestation (`-v $HOME/.docker:/root/.docker:ro` or set `DOCKER_CONFIG`).

### Kubernetes (Helm)

```bash
helm install sigward ./charts/sigward \
  --namespace sigward \
  --create-namespace \
  --set image.tag=<tag>
```

For private registries, create a `kubernetes.io/dockerconfigjson` secret and set
`registryAuth.existingSecret` so Sigward can authenticate when fetching SBOMs.

### CLI Flags

| Flag | Default | Description |
|---|---|---|
| `--enforce` | `true` | Enforce (block) vs audit mode |
| `--verbose` / `-v` | `false` | Debug logging |
| `--filter-namespaces` | `""` | Comma-separated K8s namespaces to monitor (`!` prefix to exclude). The `sigward` namespace is always excluded. |
| `--filter-image-digest` | `""` | Filter out containers running this image digest from monitoring |
| `--exempt-label` | `sigward.dev/exempt` | Kubernetes label key used to mark namespaces as exempt (value must be `true`). Evaluated at startup only. Set to `""` to disable. |

## Preparing images

Sigward verifies against a **signed SPDX SBOM attached to the image as a cosign attestation**.
Generate and attach one, for example with [syft](https://github.com/anchore/syft) and
[cosign](https://github.com/sigstore/cosign):

```bash
syft <image> -o spdx-json > sbom.spdx.json
cosign attest --predicate sbom.spdx.json --type spdxjson <image>
```

## Requirements

- Linux kernel 5.18+
- BPF LSM enabled (`CONFIG_BPF_LSM=y`, boot with `lsm=...,bpf`)
- IMA enabled (`CONFIG_IMA=y`) — required for `bpf_ima_file_hash`
- Container images with a signed SPDX SBOM (e.g. a cosign SBOM attestation)

## Development

Requires [`ig`](https://inspektor-gadget.io/docs/latest/quick-start#linux) CLI v0.49+ for
building the gadget.

```bash
# Build everything (gadget + binary). Requires sudo.
make build-all

# Run tests
make test
```

## Status

Sigward is being extracted from [Micromize](https://github.com/micromize-dev/micromize) and is
under active construction. Layout and interfaces will change.

## License

Sigward's userspace code is licensed under the [Apache License 2.0](LICENSE). The BPF code
templates are licensed under [GPL-2.0 with the Linux-syscall-note](LICENSE-bpf.txt).
