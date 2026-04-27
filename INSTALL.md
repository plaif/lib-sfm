# libSFM Installation & Verification Guide

> A step-by-step guide for installing the **man_libSFM** `.deb` package on a Linux system  
> and verifying dependencies, installed file layout, and runtime linking.

---

## Table of Contents

1. [Installing the Package](#1-installing-the-package)
2. [Verifying Dependencies](#2-verifying-dependencies)
3. [Listing Installed Files](#3-listing-installed-files)
4. [Checking Paths on Disk](#4-checking-paths-on-disk)
5. [Verifying Runtime Links](#5-verifying-runtime-links)
6. [Installation Checklist](#6-installation-checklist)

---

## 1. Installing the Package

### ✅ Recommended — install via `apt`

Unlike `dpkg -i`, `apt` automatically resolves and installs all packages listed in `Depends`.

```bash
sudo apt install ./man_libSFM_<version>_amd64.deb
```

### Alternative — install via `dpkg`, then fix dependencies

If you already installed with `dpkg -i`, run the following to resolve any missing dependencies afterwards.

```bash
sudo dpkg -i ./man_libSFM_<version>_amd64.deb
sudo apt -f install
```

> ⚠️ Skipping the `-f install` step may leave the library in a broken state.

---

## 2. Verifying Dependencies

### Before installation — inspect the package file directly

```bash
dpkg-deb -I ./man_libSFM_<version>_amd64.deb
```

Look for the `Depends:` line in the output.

### After installation — check the system-registered metadata

```bash
dpkg -s man_libSFM
```

### Expected dependencies for the current branch

The following packages should appear in the `Depends:` field.

| Package | Purpose |
|---------|---------|
| `libc6` | C standard runtime library |
| `libstdc++6` | C++ standard library |
| `libgcc-s1` | GCC runtime support |
| `zlib1g` | Data compression library |
| `libcurl4` | HTTP/HTTPS transfer library |
| `libssl3` | OpenSSL TLS/SSL encryption |
| `libfmt8` | A fast and safe C++ formatting library (version 8) that serves as a modern, user-friendly alternative to printf |

---

## 3. Listing Installed Files

Use `dpkg -L` to list every file path deployed by the package.

```bash
dpkg -L man_libSFM
```

### Key paths to verify

| Path | Contents |
|------|----------|
| `/usr/lib/` | Shared library (`.so`) |
| `/usr/include/libsfm/` | Header files (`.h`) |
| `/usr/lib/cmake/` | CMake config files |
| `/usr/share/doc/man_libSFM/examples/` | Example source code |

---

## 4. Checking Paths on Disk

Confirm that the example sources were actually installed.

```bash
ls -al /usr/share/doc/man_libSFM/examples
```

To list only files (no subdirectories):

```bash
find /usr/share/doc/man_libSFM/examples -maxdepth 1 -type f
```

---

## 5. Verifying Runtime Links

Even after a successful install, a shared library dependency may be missing at runtime.  
Use `ldd` to check before running anything.

```bash
# Generic path
ldd /usr/lib/libsfm.so

# Architecture-specific path (amd64)
ldd /usr/lib/x86_64-linux-gnu/libsfm.so
```

### Expected output (healthy state)

```
    linux-vdso.so.1 => (0x00007ffd...)
    libssl.so.3 => /lib/x86_64-linux-gnu/libssl.so.3
    libcurl.so.4 => /usr/lib/x86_64-linux-gnu/libcurl.so.4
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6
```

### If `not found` appears

Any entry marked `not found` is a library missing at runtime and must be installed separately.

```bash
# Search for the missing library
apt-cache search <library-name>

# Install the corresponding package
sudo apt install <package-name>
```

---

## 6. Installation Checklist

All items must pass for libSFM to be correctly installed.

- [ ] Installed successfully with `sudo apt install ./man_libSFM_<version>_amd64.deb`
- [ ] `dpkg -s man_libSFM` shows all 6 expected packages in the `Depends` field
- [ ] `dpkg -L man_libSFM` lists library, header, cmake, and examples paths
- [ ] `ls /usr/share/doc/man_libSFM/examples` shows example files present
- [ ] `ldd /usr/lib/.../libsfm.so` reports no `not found` entries

##

---
## 7. Configure Your Environment

Update your `.env` file with the provided activation key and the latest model weight filename:

```env
LIBSFM_API_KEY=your-activation-key-here
LIBSFM_WEIGHT_FILE=your-weight-filename-here
```

---
## 8. Export TensorRT Path
```
export PATH="/usr/local/bin:/usr/local/cuda/bin:${PATH}"
export LD_LIBRARY_PATH="/usr/local/cuda/lib64:/usr/local/tensorrt/lib:${LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH="/usr/local/tensorrt/targets/x86_64-linux-gnu/lib:${LD_LIBRARY_PATH}"
```
