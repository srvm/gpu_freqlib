#pragma once

namespace freqlib {
  using freqlist_t = std::vector<double>;

  struct instance {
    instance() {
    }

    freqlist_t get_supported_clocks() {
    }

    freqlist_t get_supported_mem_clocks() {
    }
  };

} // ns freqlib
