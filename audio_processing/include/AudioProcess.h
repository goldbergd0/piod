#pragma once

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
    AudioProcess(): 
        m_num_channels(2),
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
    void add_process_callback(const std::string& key, const std::function<void(const AudioProcess*)>& cb) {m_callbacks[key] = cb;}
    void remove_process_callback(const std::string& key) {
        if (m_callbacks.find(key) != m_callbacks.end()) {
            m_callbacks.erase(key);
        }
    }
    // template <typename DurationType>
    // void set_history_size(const DurationType& dur) { set_history_size(std::round(dur/m_period)); }
    void stop() {
        m_stop = true;
        if (m_processing_thread.joinable()) {
            m_cond_var.notify_all();
            m_processing_thread.join();
            m_processing_thread = std::thread();
        }
    }
    void start() {m_stop = false;}
protected:
    bool detect_beat(const std::vector<int16_t>& audio_data);
    void compute_fft(const std::vector<int16_t>& audio_data);
    void on_beat();
protected:
    std::atomic_bool m_stop = false;
    std::thread m_processing_thread;
    std::vector<std::tuple<std::vector<int16_t>, std::chrono::time_point<std::chrono::high_resolution_clock>, uint64_t> > m_audio_buffer = 
        std::vector<std::tuple<std::vector<int16_t>, std::chrono::time_point<std::chrono::high_resolution_clock>, uint64_t> >(2);
    size_t m_buffer_index = 1;
    size_t m_load_buffer_index = 0;
    std::mutex m_mutex;
    std::condition_variable m_cond_var;
    std::map<std::string, std::function<void(const AudioProcess*)> > m_callbacks;
public:
    // uint32_t m_sample_rate = 44100;
    // uint32_t m_samples_per_frame = 1024;
    // // Left channel sample
    // left_channel[i] = buffer[i * 2];
    // // Right channel sample
    // right_channel[i] = buffer[i * 2 + 1];
    uint32_t m_num_channels = 2;
    // std::chrono::milliseconds m_period = std::chrono::seconds(1);
    float m_bpm = 0;
    float m_volume = 0;
    bool m_beat_detected = false;
    size_t m_fft_bins = 0;
    // vector of time, fft, volume
    std::vector<std::tuple<std::chrono::time_point<std::chrono::high_resolution_clock>, std::vector<float>, float> > m_history;
    size_t m_history_index = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_cur_time;
    std::vector<std::chrono::time_point<std::chrono::high_resolution_clock> > m_last_beat_times;
};