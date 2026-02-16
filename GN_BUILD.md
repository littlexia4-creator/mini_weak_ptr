# Building with GN

This repository now includes a standalone GN + Ninja setup.

## Targets

- `//src:mini_weak_ptr_core`: core smart-pointer module (`source_set`)
- `//src:mini_weak_ptr_samples`: sample/test helpers (`source_set`)
- `//src:mini_weak_ptr_demo`: runnable demo executable
- `//:default`: points to `//src:mini_weak_ptr_demo`

## Build

```bash
./tools/gn gen out/Default
./tools/ninja -C out/Default
```

Run the demo:

```bash
./out/Default/mini_weak_ptr_demo
```
