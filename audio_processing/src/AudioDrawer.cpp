#include <AudioDrawer.h>

#include <spdlog/spdlog.h>
#include <chrono>

using namespace std::chrono_literals;

AudioDrawer::AudioDrawer(): m_grid(16, 16) {
    m_listener.set_sample_rate(44100);
    m_listener.set_samples_per_frame(1024);
    m_listener.set_num_channels(1);
    m_listener.set_device_name("hw:0,0");
    m_process.set_history_size(50);
    m_process.add_process_callback("main", [this](const AudioProcess *proc) {
        this->update(proc);
    });
}

AudioDrawer::~AudioDrawer() {
    stop();
}

void AudioDrawer::start() {
    m_usb.open();
    m_process.start();
    m_listener.listen([this](const std::vector<int16_t>& audio_data,
                const std::chrono::time_point<std::chrono::high_resolution_clock>& timestamp,
                const uint64_t& frame_num) {
        this->m_process.queue_data(audio_data, timestamp, frame_num);
    }, 0); // 0 duration means run indefinitely until stopped
}
void AudioDrawer::stop() {
    m_listener.stop();
    m_process.stop();
}

void AudioDrawer::update(const AudioProcess *process) {
    spdlog::debug("GOT UPDATE, WRITING DATA");
    for (size_t x = 0; x < m_grid.width(); ++x) {
        for (size_t y = 0; y < m_grid.height(); ++y) {
            auto cur = m_grid.get(x, y);
            m_grid.set(x, y, std::get<0>(cur) + 1, std::get<1>(cur) + 1, std::get<2>(cur) + 1);
            spdlog::debug("({}, {}): ({}, {}, {})", x, y, std::get<0>(cur), std::get<1>(cur), std::get<2>(cur));
        }
    }
    spdlog::debug("Writing data to USB device");
    m_usb.write_and_reopen(m_grid.data());
}
