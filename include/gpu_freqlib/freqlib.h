#pragma once

#include <cstdio>
#include <map>
#include <nvml.h>

#define NVML_TRY(call)                                             \
  do {                                                             \
    nvmlReturn_t result = call;                                    \
    if(NVML_SUCCESS != result) {                                   \
      fprintf(stderr, "[Warning {%s:%d}] NVML call failed with error: %s\n",  \
              __FILE__, __LINE__, nvmlErrorString(result));                            \
    }                                                              \
  } while(0)

#define NVML_CALL(call)                                            \
  do {                                                             \
    nvmlReturn_t result = call;                                    \
    if(NVML_SUCCESS != result) {                                   \
      fprintf(stderr, "[Error {%s:%d}] NVML call failed with error %s\n",  \
              __FILE__, __LINE__, nvmlErrorString(result));                            \
      result = nvmlShutdown();                                     \
      if(NVML_SUCCESS != result) {                                 \
        fprintf(stderr, "[Error] Failed to shutdown NVML: %s\n",   \
                nvmlErrorString(result));                          \
      }                                                            \
      exit(-1);                                                    \
    }                                                              \
  } while(0)

namespace freqlib {
  using frequency_t = unsigned int;
  using freqlist_t = std::vector<frequency_t>;
  using clock_map_t = std::map<frequency_t, freqlist_t>;
  using error_t = void;

  struct instance {
    instance(unsigned int device = 0) :
      m_device_idx(device),
      m_clock_map{} {
      nvmlReturn_t result = nvmlInit();
      if(NVML_SUCCESS != result) {
        fprintf(stderr, "[Error] Failed to initialize NVML: %s\n",
                nvmlErrorString(result));
        //exit(-1);
      }
      _set_device(m_device_idx);

      // XXX: Is this call needed?
      reset_all_clocks();

      auto mem_clocks = get_supported_mem_clocks();
      for(auto clock: mem_clocks) {
        freqlist_t sm_clocks = get_supported_clocks(clock);
        m_clock_map[clock] = sm_clocks;
      }

      _print_supported_clocks();
    }

    ~instance() {
      reset_all_clocks();

      nvmlReturn_t result = nvmlShutdown();
      if(NVML_SUCCESS != result) {
        fprintf(stderr, "[Error] Failed to shutdown NVML: %s\n",
                nvmlErrorString(result));
      }
    }

    freqlist_t get_supported_clocks(frequency_t mem_clock = 0) {

      if(mem_clock == 0)
        mem_clock = get_current_mem_clock();

      unsigned int count = 0;
      NVML_TRY(nvmlDeviceGetSupportedGraphicsClocks(m_device,
               mem_clock, &count, nullptr));

      frequency_t supported[count];
      NVML_TRY(nvmlDeviceGetSupportedGraphicsClocks(m_device,
               mem_clock, &count, supported));

      return freqlist_t{supported, supported + count};
    }

    freqlist_t get_supported_mem_clocks() {
      unsigned int count = 0;
      NVML_TRY(nvmlDeviceGetSupportedMemoryClocks(m_device,
               &count, nullptr));

      frequency_t supported[count];
      NVML_TRY(nvmlDeviceGetSupportedMemoryClocks(m_device,
               &count, supported));

      return freqlist_t{supported, supported + count};
    }

    clock_map_t get_supported_clock_pairs() {
    }


    error_t step_up_clock() {
    }

    error_t step_down_clock() {
    }

    error_t step_up_mem_clock() {
    }

    error_t step_down_mem_clock() {
    }

    error_t set_clocks(frequency_t sm_clock, frequency_t mem_clock) {
    }

    frequency_t get_current_clock() {
      frequency_t clock = 0;

      NVML_TRY(nvmlDeviceGetApplicationsClock
               (m_device, NVML_CLOCK_SM, &clock));

      return clock;
    }

    frequency_t get_current_mem_clock() {
      frequency_t clock = 0;

      NVML_TRY(nvmlDeviceGetApplicationsClock
               (m_device, NVML_CLOCK_MEM, &clock));

      return clock;
    }

    error_t reset_all_clocks() {
      NVML_CALL(nvmlDeviceResetApplicationsClocks
               (m_device));
    }

  private:
    error_t _set_device(int index) {
      m_device_idx = index;
      NVML_CALL(nvmlDeviceGetHandleByIndex
                (m_device_idx, &m_device));
    }

    void _print_supported_clocks() {
      for(auto& k: m_clock_map) {
        for(auto& v: k.second) {
          printf("<%u, %u>\n", k.first, v);
        }
      }
    }

  private:
    int m_device_idx;
    nvmlDevice_t m_device;
    clock_map_t m_clock_map;
  };

} // ns freqlib
