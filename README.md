# NyroxPC

NyroxPC is an independent operating system project targeting x86_64 with a custom UEFI bootloader and kernel.

## Current status

- [x] UEFI bootloader source
- [x] ELF64 kernel loader
- [x] x86_64 kernel source
- [x] ELF linker layout with page-aligned segments
- [x] UEFI application compilation and linking
- [ ] Verified boot in QEMU
- [ ] Memory management
- [ ] Interrupt handling and scheduling
- [ ] Task management
- [ ] System interface
- [ ] ARM64 support

**Important:** The project is under active development. Successful compilation does not yet mean successful boot.

## Architecture

- Target: x86_64
- Boot: UEFI
- Kernel format: ELF64
- Bootloader format: PE32+ EFI application
- Language: C and x86 assembly

## Source layout

- `bootloader/` — UEFI bootloader
- `kernel/` — kernel source and linker script
- `include/` — shared headers
- `boot.asm` — assembly boot source

## Build status

See the GitHub Actions workflow for automated build checks.
