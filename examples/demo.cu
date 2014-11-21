#include <iostream>

#include <gpu_freqlib/freqlib.h>

int main() {

  freqlib::instance knob;

  std::vector<double> clock_frequencies{knob.get_supported_clocks()};
  std::vector<double> mem_frequencies{knob.get_supported_mem_clocks()};

  return 0;
}
