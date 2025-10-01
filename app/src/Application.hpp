#include <iostream>
#include <audio_processing.h>
#include <ArgParse.h>

class Application {
public:

    void handleArgs(int argc, char* argv[]) {
        parser.on("example", [](const std::string& value) {
            std::cout << "Example option value: " << value << std::endl;
        }, false, "An example option");
        parser.parse(argc, argv);
    }

    void run(int argc, char* argv[]) {
        this->handleArgs(argc, argv);
        std::cout << "Application is running..." << std::endl;
            std::cout << "Hello, World!" << std::endl;
    std::cout << "2 + 3 = " << audio_processing::add(2, 3) << std::endl;

    }

    void waitForExit() {
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
    }

public:
     ArgParse parser;    
};