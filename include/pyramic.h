#ifndef __PYRAMIC_H__
#define __PYRAMIC_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>

#include "pyramicio.h"

#define PYRAMIC_CHANNELS_IN 48
#define PYRAMIC_CHANNELS_OUT 2
#define PYRAMIC_SAMPLERATE 48000

// We wait until a sufficient number of ready buffers are available before
// starting playback
#define PLAY_MIN_BUF_IN_Q 3

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
    std::mutex mutex_read_ready; // protects read queue
    std::mutex mutex_read_empty; // protects read queue
    std::mutex mutex_play_ready; // protects play queue
    std::mutex mutex_play_empty; // protects play queue
    std::queue<buffer_t *> q_read_ready;  // store buffers ready to be played
    std::queue<buffer_t *> q_read_empty;  // store unused buffers
    std::queue<buffer_t *> q_play_ready;  // store buffers ready to be played
    std::queue<buffer_t *> q_play_empty;  // store unused buffers

    Pyramic(int _num_samples);
    ~Pyramic();

    int start();
    void stop();

    inline size_t n_samples() { return num_samples; }
    inline size_t channels_in() { return PYRAMIC_CHANNELS_IN; }
    inline size_t channels_out() { return PYRAMIC_CHANNELS_OUT; }
    inline size_t samplerate() { return PYRAMIC_SAMPLERATE; }

    inline bool read_available()
    {
      bool ret;
      this->mutex_read_ready.lock();
      ret = !this->q_read_ready.empty();
      this->mutex_read_ready.unlock();
      return ret;
    }
    inline buffer_t &read_pop()
    {
      this->mutex_read_ready.lock();
      buffer_t &buf = *this->q_read_ready.front();
      this->q_read_ready.pop();
      this->mutex_read_ready.unlock();
      return buf;
    }
    inline void read_push(buffer_t &buf) {
      this->mutex_read_empty.lock();
      this->q_read_empty.push(&buf);
      this->mutex_read_empty.unlock();
    }

    inline bool play_available()
    {
      bool ret;
      this->mutex_play_empty.lock();
      ret = !this->q_play_empty.empty();
      this->mutex_play_empty.unlock();
      return ret;
    }
    inline buffer_t &play_pop()
    {
      this->mutex_play_empty.lock();
      buffer_t &buf = *this->q_play_empty.front();
      this->q_play_empty.pop();
      this->mutex_play_empty.unlock();
      return buf;
    }
    inline void play_push(buffer_t &buf) {
      this->mutex_play_ready.lock();
      this->q_play_ready.push(&buf);
      this->mutex_play_ready.unlock();
    }

    // The functions to be launched in threads
    void reader();
    void player();
};

#endif // __PYRAMIC_H__
