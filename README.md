# 3DGS-Vulkan
This is a 3DGS implementation with vulkan backend, which uses hardware graphics pipeline, instead of compute pipeline with tiling.

# Preparation before build
## Dependency
+ Vulkan 1.3
+ Visual Studio 2019/2022
+ (Optional)Android Studio 2023 with SDK version 34 and NDK version 25(other version might work, not checked)
## Code
You need to pull submodules of this project:
```
git clone --recursive https://github.com/PV-NTR/3DGS-Vulkan.git
```

or

```
git clone https://github.com/PV-NTR/3DGS-Vulkan.git
git submodule init
git submodule update
```

# Build
## windows
```
cmake -B build .
```

## Android
open directory `example/android` with Android Studio.

# TODO List:
- [x] Simple Vulkan Resource Manager.
- [x] Gaussian Graphics Pipeline.
- [ ] GPU Radix Sort.
- [ ] Complete Camera System.
- [ ] Complete Asset System.
- [ ] UI Overlay.
- [ ] Smarter 3rdparty Dependency.
- [ ] Compute Pipeline.
- [ ] Data Compression.
- [ ] LoD Model Pipeline.

# Reference
## Paper
[3D Gaussian Splatting for Real-Time Radiance Field Rendering](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/).
## Coding
+ Vulkan Project: https://github.com/SaschaWillems/Vulkan
+ Gaussian Splatting: https://github.com/huggingface/gsplat.js
