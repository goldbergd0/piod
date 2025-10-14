#include <AudioListener.h>
#include <alsa/asoundlib.h>
#include <spdlog/spdlog.h>
#include <fmt/chrono.h>

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

AudioListener::~AudioListener() {
    stop();
}

int AudioListener::listen(
    const std::function<void(
        const std::vector<int16_t>&,
        const std::chrono::time_point<std::chrono::high_resolution_clock> &,
        const uint64_t&)>& callback,
    int duration_seconds) {
    if (m_listener_thread.joinable()) {
        spdlog::warn("Listener is already running.");
        return -1;
    } else {
        m_stop_flag.store(false);
    }
    
    int rc;
    int dir;
    
    // Open PCM device for recording (capture).
    // Replace "hw:1,0" with your specific card and device numbers if different.
    rc = snd_pcm_open(&m_handle, m_device_name.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        std::cerr << "unable to open pcm device: " << snd_strerror(rc) << std::endl;
        return 1;
    }
    spdlog::info("PCM device {} opened for recording.", m_device_name);

    // Allocate a hardware parameters object.
    snd_pcm_hw_params_alloca(&m_params);

    // Fill it with default values.
    snd_pcm_hw_params_any(m_handle, m_params);

    // Set the desired hardware parameters.
    // Interleaved mode
    snd_pcm_hw_params_set_access(m_handle, m_params, SND_PCM_ACCESS_RW_INTERLEAVED);

    // Signed 16-bit little-endian format
    snd_pcm_hw_params_set_format(m_handle, m_params, SND_PCM_FORMAT_S16_LE);

    // 2 channels (stereo)
    snd_pcm_hw_params_set_channels(m_handle, m_params, m_num_channels);

    snd_pcm_hw_params_set_rate_near(m_handle, m_params, &m_sample_rate, &dir);

    // Set period size (frames per period)
    snd_pcm_uframes_t frames = m_samples_per_frame;
    snd_pcm_hw_params_set_period_size_near(m_handle, m_params, &frames, &dir);


    spdlog::info("Audio parameters set: rate={} Hz, channels={}, format=S16_LE", m_sample_rate, m_num_channels);
    // Write the parameters to the driver
    rc = snd_pcm_hw_params(m_handle, m_params);
    if (rc < 0) {
        std::cerr << "unable to set hw parameters: " << snd_strerror(rc) << std::endl;
        return -1;
    }

    // Get period size
    snd_pcm_hw_params_get_period_size(m_params, &frames, &dir);
    m_samples_per_frame = frames;

    /* Allocate a temporary swparams struct */
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);

    /* Retrieve current SW parameters. */
    snd_pcm_sw_params_current(m_handle, swparams);

    /* Change software parameters. */
    snd_pcm_sw_params_set_tstamp_mode(m_handle, swparams, SND_PCM_TSTAMP_ENABLE);
    snd_pcm_sw_params_set_tstamp_type(m_handle, swparams, SND_PCM_TSTAMP_TYPE_GETTIMEOFDAY);

    /* Apply updated software parameters to PCM interface. */
    snd_pcm_sw_params(m_handle, swparams);

    // Get buffer size in bytes
    // 1 int16_t per sample (S16_LE) * 2 channels
    std::vector<int16_t> buffer(frames * m_num_channels);

    m_listener_thread = std::thread([=, this]() mutable {
        // Loop for capturing audio data
        long loops = duration_seconds * (this->m_sample_rate / (float)frames);
        if (duration_seconds <= 0) {
            loops = 1;
        }
        spdlog::info("Starting audio capture for {} periods...", loops);
        while (loops > 0 && !this->m_stop_flag.load()) {
            if (duration_seconds > 0) {
                loops--;
            }
            spdlog::debug("Asking for {} frames of audio data", frames);
            rc = snd_pcm_readi(this->m_handle, buffer.data(), frames);
            auto read_time = std::chrono::high_resolution_clock::now();
            if (rc == -EPIPE) {
                // EPIPE means overrun
                spdlog::error("Overrun occurred");
                snd_pcm_prepare(this->m_handle);
            } else if (rc < 0) {
                std::string err_msg = snd_strerror(rc);
                spdlog::error("Error from read: {}", err_msg);
            } else if (rc != (int)frames) {
                spdlog::warn("short read, read {} frames", rc);
            }

            snd_htimestamp_t ts;
            snd_pcm_uframes_t num_frames;
            rc = snd_pcm_htimestamp(this->m_handle, &num_frames, &ts);
            if (rc < 0)
            {
                spdlog::warn("Unable to get timestamp: {}", snd_strerror(rc));
                return;
            }
            if (ts.tv_sec != 0) {
                auto d = std::chrono::seconds{ts.tv_sec}
                        + std::chrono::nanoseconds{ts.tv_nsec};
                read_time = std::chrono::time_point<std::chrono::high_resolution_clock>(d);
            } else {
                spdlog::warn("Timestamp is zero, using current time.");
            }

            spdlog::debug("Captured {} frames of audio data: num_frames {} time: {}", rc, num_frames, read_time);
            callback(buffer, read_time, num_frames);
        }
    });
    return 0;
}

void AudioListener::stop() {
    if (m_listener_thread.joinable()) {
        m_stop_flag.store(true);
        m_listener_thread.join();
        m_stop_flag.store(false);
    }
    m_listener_thread = std::thread();
    if (m_handle) {
        snd_pcm_drain(m_handle);
        snd_pcm_close(m_handle);
        m_handle = nullptr;
    }
}

void AudioListener::block_until_stopped() {
    if (m_listener_thread.joinable()) {
        m_listener_thread.join();
    }
}