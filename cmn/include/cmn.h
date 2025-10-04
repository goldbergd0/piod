#include <string>

namespace cmn {
    void configure_logging(const std::string& log_file = "", size_t max_size = 1048576 * 5, size_t max_files = 10);    
}
