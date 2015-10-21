GPU Frequency Library (gpu\_freqlib)
====================================

A lightweight C++ library for varying core and memory frequencies on NVIDIA GPUs.

### Pre-reqs

* NVIDIA GPU that supports core and memory frequency adjustments
* CUDA toolkit 6.5 or above
* NVML library ([download](https://developer.nvidia.com/nvidia-management-library-nvml))

### Installation

gpu\_freqlib is a header-only C++ library. So no installation is required.

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
