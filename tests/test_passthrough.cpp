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
  uint32_t duration = 5000;
  uint32_t samp_count = (uint32_t) ((duration / 1000.) * 48000 * 48);
	struct pyramic* p = pyramicInitializePyramic();

	if(p) {
		printf("Success in Initializing Pyramic !\n");

		// Get output buffer and fill with zeros silence
		struct outputBuffer outBuf = pyramicGetOutputBuffer(p, 2*samp_count);
    for (i = 0 ; i < outBuf.length ; i++)
      outBuf.samples[i] = 0;
		pyramicSelectOutputSource(p, SRC_MEMORY);

    // Start capture
		pyramicStartCapture(p, duration);

		struct inputBuffer inBuf = pyramicGetInputBuffer(p, 0); // 0 for 1st half, but we can indeed get all samples

		usleep(200);
		while(pyramicGetCurrentBufferHalf(p) != 0) {

		}
		printf("5s capture finished\n");
    printf("N samples / mic: %d\n", inBuf.samplesPerMic);

		for(i = 0; i < inBuf.samplesPerMic; i++) { // two halves
			outBuf.samples[2*i] = inBuf.samples[48*i]; // L microphone 6
			outBuf.samples[2*i+1] = inBuf.samples[48*i+1]; // R
      printf("%d %d\n", (int)outBuf.samples[2*i], (int)outBuf.samples[2*i+1]);
		}
		printf("Output buffer filled !\n");

		pyramicSetOutputBuffer(p, outBuf);
		pyramicSelectOutputSource(p, SRC_MEMORY);
		printf("Source selected!\n");

		usleep(200);
		while(pyramicGetCurrentBufferHalf(p) != 0) {

		}
		printf("5s playback finished\n");
    for (i = 0 ; i < outBuf.length ; i++)
      outBuf.samples[i] = 0;

		pyramicStopCapture(p);
	}
	else
		printf("Failed to init Pyramic !\n");

	return 0;
}
