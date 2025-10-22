#include <AudioProcess.h>
#include <audio_processing.h>
#include "spdlog/spdlog.h"
#include <fmt/chrono.h>
#include <numeric>

using tp = std::chrono::time_point<std::chrono::high_resolution_clock>;

void AudioProcess::stop() {
    m_stop = true;
    if (m_processing_thread.joinable()) {
        m_cond_var.notify_all();
        m_processing_thread.join();
        m_processing_thread = std::thread();
    }
    m_listener.stop();
}

void AudioProcess::start() {
    m_stop = false;
    m_listener.listen([this](const std::vector<int16_t>& audio_data,
            const std::chrono::time_point<std::chrono::high_resolution_clock>& timestamp,
            const uint64_t& frame_num) {
        this->queue_data(audio_data, timestamp, frame_num);
    }, 0); // 0 duration means run indefinitely until stopped
}

void AudioProcess::queue_data(const std::vector<int16_t>& audio_data,
            const tp& timestamp,
            const uint64_t& frame_num) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_load_buffer_index = (m_load_buffer_index + 1) % m_audio_buffer.size();
    if (m_load_buffer_index == m_buffer_index) {
        spdlog::warn("Audio buffer overrun, overwriting unprocessed data.");
        m_load_buffer_index = (m_load_buffer_index + 1) % m_audio_buffer.size();
    }
    auto& load = m_audio_buffer[m_load_buffer_index];
    auto& aud_buf = std::get<std::vector<int16_t> >(load);
    if (aud_buf.size() != audio_data.size()) {
        spdlog::warn("Resizing audio buffer ({}) from {} to {}", m_load_buffer_index, aud_buf.size(), audio_data.size());
        aud_buf.resize(audio_data.size());
    }
    std::get<tp>(load) = timestamp;
    std::get<uint64_t>(load) = frame_num;
    if (!m_processing_thread.joinable() && !m_stop) {
        m_processing_thread = std::thread([this]() {
            while (!m_stop) {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cond_var.wait(lock, [this]() {
                    return m_load_buffer_index != m_buffer_index || m_stop;
                });
                m_buffer_index = m_load_buffer_index;
                auto& data = m_audio_buffer[m_buffer_index];
                lock.unlock();
                process(std::get<std::vector<int16_t> >(data), std::get<tp>(data), std::get<uint64_t>(data));
            }
        });
        // NOTE: could be a small race condition here if the thread takes a while to startup...
        // If this seems like an issue, just add a small sleep here so the thread has time to start before the condition is notified
    }
    m_cond_var.notify_one();
}

void AudioProcess::process(const std::vector<int16_t>& audio_data,
            const tp& timestamp,
            const uint64_t& frame_num) {
    m_history_index = (m_history_index + 1) % m_history.size();
    spdlog::info("Audio data size: {}, Timestamp: {}, Frame number: {}", audio_data.size(), timestamp, frame_num);
    m_volume = std::accumulate(audio_data.begin(), audio_data.end(), 0.0f, [](float sum, int16_t val) {
            return sum + std::abs(val);
        }) / audio_data.size();
    m_cur_time = timestamp;
    compute_fft(audio_data);
    std::get<tp>(m_history[m_history_index]) = timestamp;
    std::get<float>(m_history[m_history_index]) = m_volume;
    if (detect_beat(audio_data)) {
        on_beat();
    }
    for (const auto& [key, cb] : m_callbacks) {
        cb(this);
    }
}

void AudioProcess::on_beat() {
    spdlog::info("Beat detected at {:?}!", m_cur_time);
    m_beat_detected = true;
    // TODO: set bpm
    m_bpm = 120.0f;
    m_last_beat_times.push_back(m_cur_time);
    if (m_last_beat_times.size() > m_history.size()) {
        m_last_beat_times.erase(m_last_beat_times.begin());
    }
}

bool AudioProcess::detect_beat(const std::vector<int16_t>& audio_data) {
    // Placeholder for beat detection logic
    spdlog::debug("Detecting beat in audio data...");
    return false;
}

void AudioProcess::compute_fft(const std::vector<int16_t>& audio_data) {
    spdlog::debug("Computing FFT...");
    static std::vector<float> fft_in_buffer(audio_data.size(), 0.0f);
    static std::vector<float> fft_out_buffer(audio_data.size(), 0.0f);
    if (fft_in_buffer.size() != audio_data.size()) {
        fft_in_buffer.resize(audio_data.size(), 0.0f);
        fft_out_buffer.resize(audio_data.size(), 0.0f);
    }
    
    audio_processing::fft(fft_in_buffer, fft_out_buffer);
    auto& final_buffer = std::get<std::vector<float> >(m_history[m_history_index]);
    if (final_buffer.size() != m_fft_bins) {
        final_buffer.resize(m_fft_bins, 0.0f);
    }
    audio_processing::resample(fft_out_buffer, final_buffer);
    m_fft = &final_buffer;
    spdlog::debug("FFT computed: {}", final_buffer.size());
    // Units of the bins of the DFT are: 
    //   freq = i * sample_rate / N
}
