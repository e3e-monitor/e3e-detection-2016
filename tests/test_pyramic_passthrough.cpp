#include <stdio.h>
#include <stdlib.h>
#include "pyramic.h"

#define NUM_SAMPLES 10000
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

      printf("%d %d %d\n", (int)input_samples[0], (int)input_samples[1], (int)input_samples[2]);

      if (!pyramic.play_available())
      {
        //printf("Buffer underflow at processing");
        continue;
      }

      buffer_t &output_samples = pyramic.play_pop();

      for (size_t n ; n < pyramic.n_samples() ; n++)
      {
        output_samples[pyramic.channels_out() * n] = input_samples[pyramic.channels_in() * n];
        output_samples[pyramic.channels_out() * n + 1] = input_samples[pyramic.channels_in() * n + 1];
      }

      pyramic.read_push(input_samples);
      pyramic.play_push(output_samples);

      ellapsed_time += (float)pyramic.n_samples() / pyramic.samplerate();
    }

    pyramic.stop();
  }

}
