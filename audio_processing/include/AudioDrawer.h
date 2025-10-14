#pragma once

#include <GridData.h>
#include <AudioProcess.h>
#include <AudioListener.h>
#include <Usb.h>

class AudioDrawer {
public:
    AudioDrawer();
    virtual ~AudioDrawer();
    void start();
    void stop();
    void update(const AudioProcess *process);
private:
    GridData m_grid;
    AudioProcess m_process;
    AudioListener m_listener;
    Usb m_usb;
    //m_sample_rate(44100),
    //m_samples_per_frame(1024),
    //m_period(std::chrono::milliseconds(static_cast<int>(1000.0f * m_samples_per_frame / m_sample_rate))),

};