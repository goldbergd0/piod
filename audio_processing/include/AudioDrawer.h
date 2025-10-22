#pragma once

#include <GridData.h>
#include <AudioProcess.h>
#include <AudioListener.h>
#include <Usb.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdint>

class AudioDrawer {
public:
    AudioDrawer();
    virtual ~AudioDrawer();
    void start();
    void stop();
    void update(const AudioProcess *process);
private:
    void draw_thread();
    void draw();
private:
    GridData m_grid;
    std::vector<uint8_t> m_data;
    AudioProcess m_process;
    Usb m_usb;
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_data_loaded = false;
    //m_sample_rate(44100),
    //m_samples_per_frame(1024),
    //m_period(std::chrono::milliseconds(static_cast<int>(1000.0f * m_samples_per_frame / m_sample_rate))),

};