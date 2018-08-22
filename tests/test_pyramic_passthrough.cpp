#include <stdio.h>
#include <stdlib.h>
#include "pyramic.h"

#define NUM_SAMPLES 1024
#define T_MAX 20

int main(void)
{
  int ret;
  float ellapsed_time = 0.;

  Pyramic pyramic(NUM_SAMPLES);

  ret = pyramic.start();

  if (ret)
  {
    while (ellapsed_time < T_MAX)
    {
      // wait for some samples to be available
      while (!pyramic.read_available())
        usleep(50);

      buffer_t &input_samples = pyramic.read_pop();

      // We get an output buffer
      if (!pyramic.play_available())
      {
        printf("Buffer underflow at processing");
        continue;
      }
      buffer_t &output_samples = pyramic.play_pop();

      // Copy the first two input channels to the output
      for (size_t n = 0 ; n < pyramic.n_samples() ; n++)
      {
        output_samples[pyramic.channels_out() * n] = input_samples[pyramic.channels_in() * n];
        output_samples[pyramic.channels_out() * n + 1] = input_samples[pyramic.channels_in() * n + 1];
      }

      // Push back the buffers in the queues
      pyramic.read_push(input_samples);
      pyramic.play_push(output_samples);

      // move time forward
      ellapsed_time += (float)pyramic.n_samples() / pyramic.samplerate();
    }

    pyramic.stop();
  }

}
