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

## Status

Sigward is being extracted from [Micromize](https://github.com/micromize-dev/micromize) and is
under active construction. Layout and interfaces will change.

## Requirements

- Linux kernel 5.18+
- BPF LSM enabled (`CONFIG_BPF_LSM=y`, boot with `lsm=...,bpf`)
- IMA enabled (`CONFIG_IMA=y`) — required for `bpf_ima_file_hash`
- Container images with a signed SPDX SBOM (e.g. a cosign SBOM attestation)

## License

Sigward's userspace code is licensed under the [Apache License 2.0](LICENSE). The BPF code
templates are licensed under [GPL-2.0 with the Linux-syscall-note](LICENSE-bpf.txt).
