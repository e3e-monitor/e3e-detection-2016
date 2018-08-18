/*
 * main.c
 *
 *  Created on: Dec 22, 2016
 *      Author: ferry
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pyramicio.h"

int main(void)
{
  uint32_t i;
  uint32_t duration = 100;
  uint32_t samp_count = (uint32_t) ((duration / 1000.) * 48000 * 48);
	struct pyramic* p = pyramicInitializePyramic();
  int flag_playback_running = 0;

	if(p) {
		printf("Success in Initializing Pyramic !\n");

		// Get output buffer and fill with zeros silence

    // Start capture
		pyramicStartCapture(p, duration);
    struct inputBuffer inBuf = pyramicGetInputBuffer(p, 0); // 0 for 1st half, actually a pointer to the whole buffer
    printf("Input buffer size: %d\n", (int) inBuf.samplesPerMic);
		struct outputBuffer outBuf = pyramicGetOutputBuffer(p, 2 * inBuf.samplesPerMic);
    for (i = 0 ; i < outBuf.length ; i++)
      outBuf.samples[i] = 0;

    // necessary to sync ?
    while (pyramicGetCurrentBufferHalf(p) == 1) {}

    int n = 0;
    while (n < 100)
    {
      // Wait until 1st half of buffer is idle
      while(pyramicGetCurrentBufferHalf(p) != 1) {
      }

      // fill output from input
      for(i = 0 ; i < inBuf.samplesPerMic / 2 ; i++) { // two halves
        outBuf.samples[2*i] = inBuf.samples[48*i]; // L microphone 6
        outBuf.samples[2*i+1] = inBuf.samples[48*i+1]; // R
      }

      pyramicSelectOutputSource(p, SRC_MEMORY);
      pyramicSetOutputBuffer(p, outBuf);

      while(pyramicGetCurrentBufferHalf(p) != 2) {
      }

      // fill output from input
      for(i = inBuf.samplesPerMic / 2 ; i < inBuf.samplesPerMic ; i++) { // two halves
        outBuf.samples[2*i] = inBuf.samples[48*i]; // L microphone 6
        outBuf.samples[2*i+1] = inBuf.samples[48*i+1]; // R
      }

      n++;
    }

    for (i = 0 ; i < outBuf.length ; i++)
      outBuf.samples[i] = 0;

		pyramicStopCapture(p);
	}
	else
		printf("Failed to init Pyramic !\n");

	return 0;
}
