#pragma once

#include <string>
#include <functional>
#include <vector>
#include <cstdint>
#include <atomic>

namespace audio_processing {
    int listen(const std::string& device_name = "hw:0,0", const std::function<void(const std::vector<int16_t>&)>& callback = nullptr, uint32_t sample_rate = 44100, int frames_per_buffer = 1024, int duration_seconds = 10, uint32_t num_channels = 2,  const std::atomic_bool& should_stop = false);
    int fft(std::vector<float>& input, std::vector<float>& output);

    void resample(const std::vector<float>& input, std::vector<float>& output);
};
