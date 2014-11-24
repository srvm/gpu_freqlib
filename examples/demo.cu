#include <cstdio>
#include <vector>

#include <gpu_freqlib/freqlib.h>

int main() {
  freqlib::instance knob;

  printf("Current SM clock: %u\n",
         knob.get_current_clock());
  printf("Current memory clock: %u\n",
         knob.get_current_mem_clock());

  auto sm_clocks = knob.get_supported_clocks();
  auto mem_clocks = knob.get_supported_mem_clocks();

  //knob.step_up_mem_clock();
  knob.step_down_clock();
  knob.step_down_clock();

  printf("Current SM clock: %u\n",
         knob.get_current_clock());

  //std::vector<double> clock_frequencies{knob.get_supported_clocks()};
  //std::vector<double> mem_frequencies{knob.get_supported_mem_clocks()};

  return 0;
}
