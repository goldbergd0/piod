#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>


#include <audio_processing.h>
#include <ArgParse.h>
#include "spdlog/spdlog.h"
#include <iomanip>
#include <Usb.h>

uint8_t HEADER_BYTE = 42;

class Application {
public:

    void handleArgs(int argc, char* argv[]) {
        parser.on("example", [](const std::string& value) {
            std::cout << "Example option value: " << value << std::endl;
        }, false, "An example option");
        //     void on(const std::vector<std::string>& a_option, const std::function<void(const std::string&)>& a_callback, bool a_required = false, const std::string& a_help = "");
        std::vector<std::string> verbose{"verbose", "v"};
        parser.on(verbose, [](const std::string&a_lvl) {
            int lvl = std::stoi(a_lvl);
            if (lvl < 0 || lvl > 3) {
                std::cerr << "Invalid verbosity level (DEBUG default). Use 0 (error), 1 (warn), 2 (info), or 3 (debug)." << std::endl;
                spdlog::set_level(spdlog::level::debug);
                return;
            }
            switch (lvl) {
                case 0: spdlog::set_level(spdlog::level::err); break;
                case 1: spdlog::set_level(spdlog::level::warn); break;
                case 2: spdlog::set_level(spdlog::level::info); break;
                case 3: spdlog::set_level(spdlog::level::debug); break;
            }
        }, false, "Enable verbose logging");
        parser.on("size", [this](const std::string& value) {
            this->m_sz = std::stoi(value);
            std::cout << "Size option value: " << this->m_sz << std::endl;
        }, true, "An example size option");
        parser.parse(argc, argv);
    }


    void testfft(std::vector<float>& input) {
        std::vector<float> output;
        if (audio_processing::fft(input, output) == 0) {
            spdlog::info("FFT output size: {}", output.size());
        } else {
            spdlog::error("FFT computation failed.");
        }
        std::cout << "  x: ";
        for (size_t x = 0; x < input.size(); x++) {
            std::cout << x << " ";
        }
        std::cout << std::endl;
        std::cout << " in: ";
        for (const auto& val : input) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
        std::cout << "out: ";
        for (const auto& val : output) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }

    void listen_test() {
        std::atomic_bool stop_flag(false);
        std::thread listener_thread([&stop_flag]() {
            audio_processing::listen("hw:0,0", [](const std::vector<int16_t>& data) {
                spdlog::debug("Captured {} samples", data.size());
            }, 44100, 1, 1024, 10, stop_flag);
        });

        std::this_thread::sleep_for(std::chrono::seconds(5));
        stop_flag = true;
        if (listener_thread.joinable()) {
            listener_thread.join();
        }
    }

    void fft_test() {
        std::vector<float> input(10, 0); // Example input
        std::cout<< std::setw(3);
        input[0] = 1.0f; // Impulse signal
        testfft(input);
        for (size_t x = 0; x < input.size(); ++x) {
            input[x] = static_cast<float>(sin(x));
        }
        testfft(input);
        for (size_t x = 0; x < input.size(); ++x) {
            input[x] = 10;
        }
        testfft(input);
    }

    void usb_led_test(Usb& usb_interface){
        // libusb_example();
        int num_led = this->m_sz * 3;
        int sz = num_led + 1;
        // CANT SEND DATA SMALLER THAN 7 BYTES
        if (sz < 8) sz = 8;
        std::vector<uint8_t> data(sz, 0); // Default size 4 if not set
        data[0] = HEADER_BYTE;
        int in = -1;
        bool stop = false;
        while (!stop) {
            std::cout << "R (0-255): ";
            std::cin >> in;
            for (int i = 1; i < sz; i+=3) data[i] = static_cast<uint8_t>(in);
            // data[1] = static_cast<uint8_t>(in);
            std::cout << "G (0-255): ";
            std::cin >> in;
            // data[2] = static_cast<uint8_t>(in);
            for (int i = 2; i < sz; i+=3) data[i] = static_cast<uint8_t>(in);
            std::cout << "B (0-255): ";
            std::cin >> in;
            for (int i = 3; i < sz; i+=3) data[i] = static_cast<uint8_t>(in);
            // data[3] = static_cast<uint8_t>(in);
            // std::cout << "Writing: " << (int)data[0] << " " << (int)data[1] << " " << (int)data[2] << std::endl;
            std::cout << "Writing: ";
            for (const auto& byte : data) {
                std::cout << (int)byte << " ";
            }
            std::cout << std::endl;
            usb_interface.write_and_reopen(data);
            std::cout << "Continue? (1=yes, 0=no): ";
            stop = !((std::cin >> in) && in == 1);
        }
        std::cout << std::endl;
    }

    void run(int argc, char* argv[]) {
        this->handleArgs(argc, argv);
        spdlog::info("Application is running...");
        Usb u;
        u.open();
        usb_led_test(u);
    }

    void waitForExit() {
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
    }

public:
     ArgParse parser;
     int m_sz = 0;
};