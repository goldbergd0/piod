#include <AudioDrawer.h>

#include <spdlog/spdlog.h>
#include <chrono>

using namespace std::chrono_literals;

AudioDrawer::AudioDrawer(): m_grid(16, 16) {
    m_process.set_sample_rate(44100);
    m_process.set_samples_per_frame(1024);
    m_process.set_num_channels(1);
    m_process.set_device_name("hw:0,0");
    m_process.set_history_size(50);
    m_process.add_process_callback("main", [this](const AudioProcess *proc) {
        this->update(proc);
    });
}

AudioDrawer::~AudioDrawer() {
    stop();
}

void AudioDrawer::draw_thread() {
    while (!m_process.m_stop) {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cv.wait(lk, [this]{ return this->m_data_loaded; });
        m_data_loaded = false;
        if (!m_process.m_stop) {
            m_usb.write_and_reopen(m_data);
        }
    }
}

void AudioDrawer::start() {
    m_usb.open();
    m_process.start();
    m_thread = std::thread(&AudioDrawer::draw_thread, this);
}
void AudioDrawer::stop() {
    m_process.stop();
    m_data_loaded = true;
    m_cv.notify_all();
    m_thread.join();
    m_thread = std::thread();
}

void AudioDrawer::draw() {
    spdlog::debug("Writing data to USB device");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data = m_grid.data();
    m_data_loaded = true;
    m_cv.notify_all();
}

void AudioDrawer::update(const AudioProcess *process) {
    spdlog::debug("GOT UPDATE, WRITING DATA");
}
