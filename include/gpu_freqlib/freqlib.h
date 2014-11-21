#pragma once

#include <cstdio>
#include <nvml.h>

#define NVML_CALL(call)                                            \
  do {                                                             \
    nvmlReturn_t result = call;                                    \
    if(NVML_SUCCESS != result) {                                   \
      fprintf(stderr, "[Fatal] NVML call failed with error %s\n",  \
              nvmlErrorString(result));                            \
      result = nvmlShutdown();                                     \
      if(NVML_SUCCESS != result) {                                 \
        fprintf(stderr, "[Fatal] Failed to shutdown NVML: %s\n",   \
                nvmlErrorString(result));                          \
      }                                                            \
    }                                                              \
  } while(0)

namespace freqlib {
  using freqlist_t = std::vector<double>;
  using error_t = void;
  using frequency_t = double;

  struct instance {
    instance() :
      m_device(0) {
      nvmlReturn_t result = nvmlInit();
      if(NVML_SUCCESS != result) {
        fprintf(stderr, "[Fatal] Failed to initialize NVML: %s\n",
                nvmlErrorString(result));
        //exit(-1);
      }
    }

    ~instance() {
      nvmlReturn_t result = nvmlShutdown();
      if(NVML_SUCCESS != result) {
        fprintf(stderr, "[Fatal] Failed to shutdown NVML: %s\n",
                nvmlErrorString(result));
      }
    }

    freqlist_t get_supported_clocks() {
    }

    freqlist_t get_supported_mem_clocks() {
    }

    error_t step_up_clock() {
    }

    error_t step_down_clock() {
    }

    error_t step_up_mem_clock() {
    }

    error_t step_down_mem_clock() {
    }

    error_t set_clock(frequency_t value) {
    }

    error_t set_mem_clock(frequency_t value) {
    }

    frequency_t get_current_clock() {
    }

    frequency_t get_current_mem_clock() {
    }

  private:
    int m_device;
  };

} // ns freqlib
