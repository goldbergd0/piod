#include <iostream>
#include <vector>
#include <string>

#include <audio_processing.h>
#include <ArgParse.h>
#include "spdlog/spdlog.h"
#include <iomanip>

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
        parser.parse(argc, argv);
    }


    void test(std::vector<float>& input) {
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

    void run(int argc, char* argv[]) {
        this->handleArgs(argc, argv);
        spdlog::info("Application is running...");
        audio_processing::listen("hw:0,0", [](const std::vector<int16_t>& data) {
            // Example callback processing
            spdlog::debug("Received audio data of size: {}", data.size());
        });

        std::vector<float> input(10, 0); // Example input
        std::cout<< std::setw(3);
        input[0] = 1.0f; // Impulse signal
        test(input);
        for (size_t x = 0; x < input.size(); ++x) {
            input[x] = static_cast<float>(sin(x));
        }
        test(input);
        for (size_t x = 0; x < input.size(); ++x) {
            input[x] = 10;
        }
        test(input);

        std::cout << std::endl;
    }

    void waitForExit() {
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
    }

public:
     ArgParse parser;    
};