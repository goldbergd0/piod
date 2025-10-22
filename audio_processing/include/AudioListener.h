#pragma once

#include <string>
#include <functional>
#include <vector>
#include <cstdint>
#include <atomic>
#include <thread>
#include <chrono>

struct _snd_pcm;
typedef struct _snd_pcm snd_pcm_t;
struct _snd_pcm_hw_params;
typedef struct _snd_pcm_hw_params snd_pcm_hw_params_t;

class AudioListener {
public:
    AudioListener(const std::string& device_name = "hw:0,0", uint32_t sample_rate = 44100, uint32_t samples_per_frame = 1024, uint32_t num_channels = 2):
        m_sample_rate(sample_rate),
        m_samples_per_frame(samples_per_frame),
        m_num_channels(num_channels),
        m_device_name(device_name) {}
    ~AudioListener();
    void set_sample_rate(uint32_t rate) { m_sample_rate = rate; }
    void set_samples_per_frame(uint32_t spf) { m_samples_per_frame = spf; }
    void set_num_channels(uint32_t channels) { m_num_channels = channels; }
    void set_device_name(const std::string& name) { m_device_name = name; }
    int listen(
        const std::function<void(
            const std::vector<int16_t>&,
            const std::chrono::time_point<std::chrono::high_resolution_clock> &,
            const uint64_t&)>& callback,
        int duration_seconds = 10);
    void stop();
    void block_until_stopped();

private:
    std::atomic_bool m_stop_flag = true;
    std::thread m_listener_thread;
    snd_pcm_t *m_handle = nullptr;
    snd_pcm_hw_params_t *m_params = nullptr;
    uint32_t m_sample_rate = 44100;
    uint32_t m_samples_per_frame = 1024;
    uint32_t m_num_channels = 2;
    std::string m_device_name;

};