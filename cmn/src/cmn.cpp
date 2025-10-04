#include <cmn.h>

#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace cmn {

void configure_logging(const std::string& log_file, size_t max_size, size_t max_files) {
    // Create a logger that logs to both the console and a file.
    try {
        // auto file_logger = spdlog::rotating_logger_mt("basic_logger", log_file, max_size, max_files);
        // spdlog::set_default_logger(file_logger);
        spdlog::init_thread_pool(8192, 1);
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file, max_size, max_files);
        std::vector<spdlog::sink_ptr> sinks {stdout_sink, rotating_sink};
        auto logger = std::make_shared<spdlog::async_logger>("basic_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
        // spdlog::set_level(spdlog::level::info); // Set global log
    }
    catch (const spdlog::spdlog_ex& ex) {
        // Handle potential logger creation errors.
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

}