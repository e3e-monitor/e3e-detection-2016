/*
 * main.c
 *
 *  Created on: Dec 22, 2016
 *      Author: ferry
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <cmath>

#include "pyramicio.h"

#define BUFFER_LEN 256

int16_t sine[BUFFER_LEN];

// The Pyramic stuff
struct pyramic *p = NULL;

// Create a queue to store output frames
int is_playing = false;

int toggle_half(int half)
{ 
  if (half == 1)
    return 2;
  else
    return 1;
}

void playback()
{
  int i;
  int is_pyramic_enabled = false;

  // Get pointer to output buffer
  struct outputBuffer outBuf = pyramicGetOutputBuffer(p, 8 * BUFFER_LEN);  // 2nd arg is length in bytes (i.e., 2 buffers x 2 bytes/sample x 2 channels x buffer length)
  int16_t *out_buffers[2] = {
    outBuf.samples,
    outBuf.samples + 2 * BUFFER_LEN  // 1 buffer x 1 int16_t/sample x 2 channel x buffer length
  };

  // zero out the first buffer
  for (i = 0 ; i < 2 * 2 * BUFFER_LEN ; i++)
    outBuf.samples[i] = 0;

  int16_t *buffer;
  int current_half = 1;

  // mark as currently running
  is_playing = true;

  // configure the output
  pyramicSelectOutputSource(p, SRC_MEMORY);
  pyramicSetOutputBuffer(p, outBuf);

  // start the loop
  while (is_playing)
  {
    // Wait for current half to be idle
    while (is_pyramic_enabled && pyramicGetCurrentOutputBufferHalf(p) == current_half)
      ;

    for (i = 0 ; i < BUFFER_LEN ; i++)
    {
      out_buffers[current_half-1][2*i] = sine[i];
      out_buffers[current_half-1][2*i+1] = sine[i];
    }

    if (not is_pyramic_enabled)
    {
      pyramicEnableOutput(p, 1);
      is_pyramic_enabled = true;
    }

    current_half = toggle_half(current_half);
  }
    
  // zero out the output buffer at the end
  for (i = 0 ; i < 2 * 2 * BUFFER_LEN ; i++)
    outBuf.samples[i] = 0;

}


int main(void)
{
  uint32_t i;

  // fill the sine wave
  for (i = 0 ; i < BUFFER_LEN ; i++)
  {
    sine[i] = (int16_t)(0.01 * (2 << 15) * sin(i * 2. * M_PI / BUFFER_LEN));
  }

  // Initialize the Pyramic array
  p = pyramicInitializePyramic();

	if(p) {
		printf("Success in Initializing Pyramic !\n");

    // Start the playback too
    std::thread playback_thread(playback);

    sleep(20);

    is_playing = false;
    playback_thread.join();

		pyramicStopCapture(p);
	}
	else
		printf("Failed to init Pyramic !\n");

	return 0;
}
