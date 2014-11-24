#pragma once

#include <cstdio>
#include <map>
#include <algorithm>
#include <nvml.h>

#define NVML_TRY(call)                                             \
  do {                                                             \
    nvmlReturn_t result = call;                                    \
    if(NVML_SUCCESS != result) {                                   \
      fprintf(stderr, "[%s:%d] NVML call failed with error: %s\n",  \
              __FILE__, __LINE__, nvmlErrorString(result));                            \
    }                                                              \
  } while(0)

#define NVML_CALL(call)                                            \
  do {                                                             \
    nvmlReturn_t result = call;                                    \
    if(NVML_SUCCESS != result) {                                   \
      fprintf(stderr, "[%s:%d] NVML call failed with error %s\n",  \
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
  using clock_pair_t = std::vector<std::pair<frequency_t, frequency_t>>;
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
      // TODO: Each instance object is fixed to
      // one device. Make this more general.
      _set_device(m_device_idx);

      // XXX: Is this call needed?
      reset_all_clocks();

      m_mem_clocks = get_supported_mem_clocks();
      std::sort(m_mem_clocks.begin(), m_mem_clocks.end());
      for(auto clock: m_mem_clocks) {
        freqlist_t sm_clocks = get_supported_clocks(clock);
        std::sort(sm_clocks.begin(), sm_clocks.end());
        m_clock_map[clock] = sm_clocks;
      }

      //_print_supported_clocks();
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

    clock_pair_t get_supported_clock_pairs() {
      clock_pair_t clock_pairs;
      for(auto k: m_clock_map) {
        for(auto v: k.second) {
          clock_pairs.push_back(std::make_pair(k.first, v));
        }
      }
      return clock_pairs;
    }

    error_t step_up_clock() {
      frequency_t mem_clock = get_current_mem_clock();
      auto sm_clocks = m_clock_map[mem_clock];

      // FIXME: find may be too slow for this.
      auto clock_iterator = std::find(sm_clocks.begin(),
                                      sm_clocks.end(),
                                      get_current_clock());
      frequency_t next_clock =
        ((clock_iterator+1) != sm_clocks.end())?
        *(clock_iterator+1): *clock_iterator;

      //printf("step_up: Current is %u, Next is %u\n", *clock_iterator, next_clock);
      set_clocks(mem_clock, next_clock);
    }

    error_t step_down_clock() {
      frequency_t mem_clock = get_current_mem_clock();
      auto sm_clocks = m_clock_map[mem_clock];

      // FIXME: find may be too slow for this.
      auto clock_iterator = std::find(sm_clocks.begin(),
                                      sm_clocks.end(),
                                      get_current_clock());
      frequency_t prev_clock =
        (clock_iterator == sm_clocks.begin())?
        *clock_iterator: *(clock_iterator-1);

      //printf("step_down: Current is %u, Prev is %u\n", *clock_iterator, prev_clock);
      set_clocks(mem_clock, prev_clock);
    }

    error_t step_up_mem_clock() {
      // FIXME: find may be too slow for this.
      auto clock_iterator = std::find(m_mem_clocks.begin(),
                                      m_mem_clocks.end(),
                                      get_current_mem_clock());
      frequency_t next_clock =
        ((clock_iterator+1) != m_mem_clocks.end())?
        *(clock_iterator+1): *clock_iterator;

      //printf("step_up: Current is %u, Next is %u\n", *clock_iterator, next_clock);
      set_clocks(next_clock, get_current_clock());
    }

    error_t step_down_mem_clock() {
      // FIXME: find may be too slow for this.
      auto clock_iterator = std::find(m_mem_clocks.begin(),
                                      m_mem_clocks.end(),
                                      get_current_mem_clock());
      frequency_t prev_clock =
        (clock_iterator == m_mem_clocks.begin())?
        *clock_iterator: *(clock_iterator-1);

      //printf("step_down: Current is %u, Prev is %u\n", *clock_iterator, prev_clock);
      set_clocks(prev_clock, get_current_clock());
    }

    error_t set_clocks(frequency_t mem_clock, frequency_t sm_clock) {
      if(!is_valid_clock_pair(mem_clock, sm_clock)) {
        fprintf(stderr, "[Warning] Clock pair <%u, %u> not supported\n",
                mem_clock, sm_clock);
        return;
      }
      else {
        unsigned long long throttle_reasons;

        // Check if H/W or some other unknown throttling is enabled
        NVML_TRY(nvmlDeviceGetCurrentClocksThrottleReasons(m_device, &throttle_reasons));
        if(throttle_reasons & nvmlClocksThrottleReasonHwSlowdown)
          fprintf(stderr, "[Warning] H/W clock throttling is enabled\n");
        if(throttle_reasons & nvmlClocksThrottleReasonUnknown)
          fprintf(stderr, "[Warning] Unknown reason causing clock throttling\n");

        // Proceed to change clock rates
        NVML_TRY(nvmlDeviceSetApplicationsClocks(m_device, mem_clock, sm_clock));

        if(get_current_clock() != sm_clock)
          fprintf(stderr, "[Warning] SM clock change unsuccessful\n");
        if(get_current_mem_clock() != mem_clock)
          fprintf(stderr, "[Warning] Memory clock change unsuccessful\n");
      }
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
      for(auto v: get_supported_clock_pairs()) {
        printf("<%u, %u>\n", v.first, v.second);
      }
    }

    inline bool is_valid_clock_pair(frequency_t mem_clock,
                                    frequency_t sm_clock) {
      auto sm_clocks = m_clock_map[mem_clock];
      if(std::find(sm_clocks.begin(),
                   sm_clocks.end(),
                   sm_clock) == sm_clocks.end())
        return false;
      else
        return true;
    }

  private:
    int m_device_idx;
    nvmlDevice_t m_device;
    clock_map_t m_clock_map;
    freqlist_t m_mem_clocks;
  };

} // ns freqlib
