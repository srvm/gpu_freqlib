GPU Frequency Library (gpu\_freqlib)
====================================

A lightweight C++ library for varying core and memory clock frequencies on NVIDIA GPUs.

### Pre-reqs

* NVIDIA GPU that supports core and memory clock frequency adjustments
* CUDA toolkit 6.5 or above
* NVIDIA GDK (for NVML) ([download](https://developer.nvidia.com/gpu-deployment-kit))

### Installation

`gpu_freqlib` is a header-only C++ library. So no installation is required.

### Before You Start

Before trying to vary clock frequencies on the GPU, please ensure that persistence mode
is enabled. This allows the driver to retain the chosen frequencies across kernel calls.
Further, unless you want to run as root, make sure application clocks
can be varied by normal users.

Please refer to the following article from NVIDIA for more information:
[Increase Performance with GPU Boost and K80 Autoboost](http://devblogs.nvidia.com/parallelforall/increase-performance-gpu-boost-k80-autoboost/).

### Sample Usage

```c++
freqlib::instance knob;

printf("Current SM clock: %u\n", knob.get_current_clock());
printf("Current memory clock: %u\n", knob.get_current_mem_clock());

// Get vector of supported frequencies
auto sm_clocks = knob.get_supported_clocks();

// Step down SM (core) clock one level
knob.step_down_clock();

printf("Current SM clock: %u\n", knob.get_current_clock());
```

