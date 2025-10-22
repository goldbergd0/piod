#pragma once

#include <AudioListener.h>

#include <vector>
#include <cstdint>
#include <chrono>
#include <tuple>
#include <cmath>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <string>
#include <map>
#include <functional>


class AudioProcess {
public:
    AudioProcess(const std::string& device_name = "hw:0,0", uint32_t sample_rate = 44100, uint32_t samples_per_frame = 1024, uint32_t num_channels = 2):
        m_sample_rate(sample_rate),
        m_samples_per_frame(samples_per_frame),
        m_num_channels(num_channels),
        m_device_name(device_name),
        //   m_fft_bins(m_samples_per_frame / 2 + 1),
        m_fft_bins(512),
        m_history(5),
        m_history_index(0)
    {}
    virtual ~AudioProcess() { stop(); }
private:
    AudioProcess(const AudioProcess&) = delete;
    AudioProcess& operator=(const AudioProcess&) = delete;
    AudioProcess(AudioProcess&&) = delete;
    AudioProcess& operator=(AudioProcess&&) = delete;
public:
    void set_sample_rate(uint32_t rate) { m_sample_rate = rate; }
    void set_samples_per_frame(uint32_t spf) { m_samples_per_frame = spf; }
    void set_num_channels(uint32_t channels) { m_num_channels = channels; }
    void set_device_name(const std::string& name) { m_device_name = name; }
public:
    void stop();
    void start();
    void queue_data(const std::vector<int16_t>& audio_data,
            const std::chrono::time_point<std::chrono::high_resolution_clock>& timestamp,
            const uint64_t& frame_num);
    void process(const std::vector<int16_t>& audio_data,
            const std::chrono::time_point<std::chrono::high_resolution_clock>& timestamp,
            const uint64_t& frame_num);
    void set_history_size(size_t size) {
        m_history.resize(size);
        for (auto& entry : m_history) {
            std::get<std::vector<float> >(entry) = std::vector<float>(m_fft_bins, 0.0f);
        }
    }
    void set_num_fft_bins(size_t size) {m_fft_bins = size;}
    void add_process_callback(const std::string& key, const std::function<void(const AudioProcess*)>& cb) {m_callbacks[key] = cb;}
    void remove_process_callback(const std::string& key) {
        if (m_callbacks.find(key) != m_callbacks.end()) {
            m_callbacks.erase(key);
        }
    }
    // template <typename DurationType>
    // void set_history_size(const DurationType& dur) { set_history_size(std::round(dur/m_period)); }
protected:
    bool detect_beat(const std::vector<int16_t>& audio_data);
    void compute_fft(const std::vector<int16_t>& audio_data);
    void on_beat();
protected:
    std::thread m_processing_thread;
    std::vector<std::tuple<std::vector<int16_t>, std::chrono::time_point<std::chrono::high_resolution_clock>, uint64_t> > m_audio_buffer = 
        std::vector<std::tuple<std::vector<int16_t>, std::chrono::time_point<std::chrono::high_resolution_clock>, uint64_t> >(2);
    size_t m_buffer_index = 1;
    size_t m_load_buffer_index = 0;
    std::mutex m_mutex;
    std::condition_variable m_cond_var;
    std::map<std::string, std::function<void(const AudioProcess*)> > m_callbacks;
public:
    std::atomic_bool m_stop = false;

    uint32_t m_sample_rate;
    uint32_t m_samples_per_frame;
    uint32_t m_num_channels;
    std::string m_device_name;    // // Left channel sample
    // left_channel[i] = buffer[i * 2];
    // // Right channel sample
    // right_channel[i] = buffer[i * 2 + 1];
    float m_bpm = 0;
    float m_volume = 0;
    std::vector<float>* m_fft = nullptr;
    bool m_beat_detected = false;
    size_t m_fft_bins = 0;
    // vector of time, fft, volume
    std::vector<std::tuple<std::chrono::time_point<std::chrono::high_resolution_clock>, std::vector<float>, float> > m_history;
    size_t m_history_index = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_cur_time;
    std::vector<std::chrono::time_point<std::chrono::high_resolution_clock> > m_last_beat_times;
    AudioListener m_listener;
};