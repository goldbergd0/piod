#include <audio_processing.h>

#include <alsa/asoundlib.h>
#include "spdlog/spdlog.h"
#include "fftw3.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

int audio_processing::listen(const std::string& device_name, const std::function<void(const std::vector<int16_t>&)>& callback, uint32_t sample_rate, int frames_per_buffer, int duration_seconds, uint32_t num_channels, const std::atomic_bool& should_stop) {    
    int rc;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    int dir;
    
    // Open PCM device for recording (capture).
    // Replace "hw:1,0" with your specific card and device numbers if different.
    rc = snd_pcm_open(&handle, device_name.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        std::cerr << "unable to open pcm device: " << snd_strerror(rc) << std::endl;
        return 1;
    }
    spdlog::info("PCM device {} opened for recording.", device_name);

    // Allocate a hardware parameters object.
    snd_pcm_hw_params_alloca(&params);

    // Fill it with default values.
    snd_pcm_hw_params_any(handle, params);

    // Set the desired hardware parameters.
    // Interleaved mode
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    // Signed 16-bit little-endian format
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

    // 2 channels (stereo)
    snd_pcm_hw_params_set_channels(handle, params, num_channels);

    snd_pcm_hw_params_set_rate_near(handle, params, &sample_rate, &dir);

    // Set period size (frames per period)
    snd_pcm_uframes_t frames = frames_per_buffer;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);


    spdlog::info("Audio parameters set: rate={} Hz, channels={}, format=S16_LE", sample_rate, num_channels);
    // Write the parameters to the driver
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        std::cerr << "unable to set hw parameters: " << snd_strerror(rc) << std::endl;
        return -1;
    }

    // Get period size
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);

    // Get buffer size in bytes
    // 1 int16_t per sample (S16_LE) * 2 channels
    std::vector<int16_t> buffer(frames * num_channels);

    // Loop for capturing audio data
    long loops = duration_seconds * (sample_rate / (float)frames);
    if (duration_seconds <= 0) {
        loops = 1;
    }
    spdlog::info("Starting audio capture for {} periods...", loops);
    while (loops > 0) {
        if (duration_seconds > 0) {
            loops--;
        } else if (should_stop) {
            spdlog::info("Stop callback triggered, ending audio capture.");
            break;
        }
        spdlog::debug("Reading {} frames of audio data", frames);
        rc = snd_pcm_readi(handle, buffer.data(), frames);
        if (rc == -EPIPE) {
            // EPIPE means overrun
            spdlog::error("Overrun occurred");
            std::cerr << "overrun occurred" << std::endl;
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            std::string err_msg = snd_strerror(rc);
            spdlog::error("Error from read: {}", err_msg);
            std::cerr << "error from read: " << err_msg << std::endl;
        } else if (rc != (int)frames) {
            spdlog::warn("short read, read {} frames", rc);
            std::cerr << "short read, read " << rc << " frames" << std::endl;
        }

        spdlog::debug("Captured {} frames of audio data.", rc);
        if (callback) {
            callback(buffer);
        }
    }

    snd_pcm_close(handle);
    return 0;
}

// static int fft(const std::vector<int16_t>& input, std::vector<float>& output);
int audio_processing::fft(std::vector<float>& input, std::vector<float>& output) {
    // Ensure input size is a power of two
    size_t N = input.size();
    if (N % 2 != 0) {
        spdlog::error("Input size must be a power of two.");
        return -1;
    }
    output.resize(N);

    // Create FFTW plan
    fftwf_plan plan = fftwf_plan_r2r_1d(N, input.data(), output.data(), FFTW_REDFT10, FFTW_ESTIMATE);
    if (!plan) {
        spdlog::error("Failed to create FFTW plan.");
        return -1;
    }

    // Execute the FFT
    fftwf_execute(plan);

    // Destroy the plan
    fftwf_destroy_plan(plan);

    spdlog::debug("FFT computation completed.");
    return 0;
}

void audio_processing::resample(const std::vector<float>& input, std::vector<float>& output) {
    auto ratio = static_cast<float>(input.size()) / static_cast<float>(output.size());
    for (size_t i = 0; i < output.size(); ++i) {
        // linearly interpolate
        auto in_i = ratio * (ratio > 1.f ? i + 0.5f : i - 0.5f);
        auto before_ratio = in_i - std::floor(in_i);
        auto after_ratio = 1.0f - before_ratio;
        if (ratio < 1) {
            after_ratio = before_ratio;
            before_ratio = 1.0f - after_ratio;
        }
        
        output[i] = input[std::max(static_cast<size_t>(std::floor(in_i)), 0UL)] * before_ratio
                  + input[std::min(static_cast<size_t>(std::ceil(in_i)), input.size() - 1)] * after_ratio;
    }
}