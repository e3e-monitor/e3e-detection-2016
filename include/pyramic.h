#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <vector>

#include "pyramicio.h"

#define CHANNELS_IN 48
#define CHANNELS_OUT 2
#define SAMPLERATE 48000

// We wait until a sufficient number of ready buffers are available before
// starting playback
#define PLAY_MIN_BUF_IN_Q 2

// Number of buffers to use in the circular queues
#define N_BUFFERS 4

// Let's use vectors for the buffers, they are quite nice
typedef std::vector<int16_t> buffer_t;

class Pyramic
{
  public:
    
    size_t num_samples;  // number of samples per channel per frame

    // The Pyramic stuff
    struct pyramic *p = NULL;

    size_t read_buffer_size;
    size_t play_buffer_size;

    // flags for communication
    bool flag_is_running = false;

    // the threads
    std::thread play_thread;
    std::thread read_thread;

    // The FIFOs to keep all the buffers
    buffer_t read_buffers[N_BUFFERS];
    buffer_t play_buffers[N_BUFFERS];
    std::queue<buffer_t *> q_read_ready;  // store buffers ready to be played
    std::queue<buffer_t *> q_read_empty;  // store unused buffers
    std::queue<buffer_t *> q_play_ready;  // store buffers ready to be played
    std::queue<buffer_t *> q_play_empty;  // store unused buffers

    Pyramic(int _num_samples);
    ~Pyramic();

    int start();
    void stop();

    inline size_t n_samples() { return num_samples; }
    inline size_t channels_in() { return CHANNELS_IN; }
    inline size_t channels_out() { return CHANNELS_OUT; }
    inline size_t samplerate() { return SAMPLERATE; }

    inline bool read_available() { return !this->q_read_ready.empty(); }
    inline buffer_t &read_pop()
    {
      buffer_t *buf = this->q_read_ready.front();
      this->q_read_ready.pop();
      return *buf;
    }
    inline void read_push(buffer_t &buf) { this->q_read_empty.push(&buf); }

    inline bool play_available() { return !this->q_play_empty.empty(); }
    inline buffer_t &play_pop()
    {
      buffer_t *buf = this->q_play_empty.front();
      this->q_play_empty.pop();
      return *buf;
    }
    inline void play_push(buffer_t &buf) { this->q_play_ready.push(&buf); }




    void reader();
    void player();
};
