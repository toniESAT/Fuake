#pragma once

#include <vector>
#include <esat/time.h>

using std::vector;

namespace fuake {
struct FPSMeter {

   size_t window_length = 1 << 8;

   vector<double> frame_timestamps;
   size_t frame_counter = 0;

   float fps;

   FPSMeter(size_t window_length) : window_length(window_length) {
      frame_timestamps.resize(window_length, 0);
   }

   void Update() {
      fps = 1000 * window_length /
            (frame_timestamps[(frame_counter + window_length - 1) % window_length] -
             frame_timestamps[frame_counter % window_length]);
      frame_timestamps[frame_counter % window_length] = esat::Time();
      frame_counter++;
   }
};

} // namespace fuake