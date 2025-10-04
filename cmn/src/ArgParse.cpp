#include <ArgParse.h>
#include <stdexcept>
#include <iostream>


bool ArgParse::hasFlag(const std::string& a_flag) const {
    return m_options.find(a_flag) != m_options.end();
}

std::string ArgParse::getValue(const std::string& a_option) const {
    auto it = m_options.find(a_option);
    if (it != m_options.end()) {
        return it->second;
    }
    return "";
}

void ArgParse::on(const std::string& a_option, const std::function<void(const std::string&)>& a_callback, bool a_required, const std::string& a_help) {
    m_callbacks[a_option] = a_callback;
    if (a_required) {
        m_required.insert({a_option});
    }
    if (!a_help.empty()) {
        m_help_texts[a_option] = a_help;
    }
}

void ArgParse::on(const std::vector<std::string>& a_option, const std::function<void(const std::string&)>& a_callback, bool a_required, const std::string& a_help) {
    for (const auto& opt : a_option) {
        m_callbacks[opt] = a_callback;
        if (!a_help.empty()) {
            m_help_texts[opt] = a_help;
        }
    }
    
    if (a_required) {
        m_required.insert(a_option);
    }
}


void ArgParse::on(const std::function<void(const std::string&)>& a_callback, bool a_required, const std::string& a_help) {
    this->on("", a_callback, a_required, a_help);
}

void ArgParse::parse(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("--", 0) == 0) { // Long option
            std::string option = arg.substr(2);
            std::string value;
            auto eq_pos = option.find('=');
            if (eq_pos != std::string::npos) {
                value = option.substr(eq_pos + 1);
                option = option.substr(0, eq_pos);
            } else if (i + 1 < argc && argv[i + 1][0] != '-') {
                value = argv[++i];
            }
            m_options[option] = value;
            auto it = m_callbacks.find(option);
            if (it != m_callbacks.end()) {
                it->second(value);
            } else {
                // No callback registered for this option
                this->printHelp();
                throw std::runtime_error("Unknown option: --" + option);
            }
        } else if (arg.rfind("-", 0) == 0 && arg.length() > 1) { // Short option
            for (size_t j = 1; j < arg.length(); ++j) {
                std::string option(1, arg[j]);
                std::string value;
                if (j == arg.length() - 1 && i + 1 < argc && argv[i + 1][0] != '-') {
                    value = argv[++i];
                }
                m_options[option] = value;
                auto it = m_callbacks.find(option);
                if (it != m_callbacks.end()) {
                    it->second(value);
                } else {
                    // No callback registered for this option
                    this->printHelp();
                    throw std::runtime_error("Unknown option: -" + option);
                }
            }
        } else {
            // Positional argument or unknown format
            m_options[arg] = "";
            auto it = m_callbacks.find("");
            if (it != m_callbacks.end()) {
                it->second(arg);
            }
        }
    }
    auto found = false;
    auto bad = false;
    for (const auto& req : m_required) {
        found = false;
        for (const auto& r : req) {
            if (m_options.find(r) != m_options.end()) {
                found = true;
                break;
            }
        }
        if (!found) {
            this->printHelp();
            std::string str_req;
            for (const auto& r : req) {
                str_req += r + ", ";
            }
            if (!str_req.empty()) {
                str_req = str_req.substr(0, str_req.length() - 2); // Remove trailing comma and space
            }
            std::cerr << "Required option missing: " << str_req << std::endl;
            bad = true;
        }
    }
    if (bad) {
        throw std::runtime_error("Missing required options");
    }
}

void ArgParse::printHelp() const {
    std::cout << "Available options:\n";
    std::set<std::string> reqs;
    for (const auto& req : m_required) {
        for (const auto& r : req) {
            reqs.insert(r);
        }
    }
    for (const auto& [option, help] : m_help_texts) {
        std::cout << "  --" << option;
        if (reqs.find(option) != reqs.end()) {
            std::cout << " (required)";
        }
        std::cout << ": " << help << "\n";
    }
}