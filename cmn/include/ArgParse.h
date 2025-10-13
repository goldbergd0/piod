#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <set>
#include <vector>

class ArgParse {
public:
    bool hasFlag(const std::string& a_flag) const;
    std::string getValue(const std::string& a_option) const;
    void on(const std::string &a_option, const std::function<void(const std::string &)> &a_callback, bool a_required = false, const std::string& a_help = "");
    void on(const std::vector<std::string>& a_option, const std::function<void(const std::string&)>& a_callback, bool a_required = false, const std::string& a_help = "");
    void on(const std::function<void(const std::string &)> &a_callback, bool a_required = false, const std::string& a_help = "");
    void parse(int argc, char* argv[]);
    void printHelp() const;

private:
    std::map<std::string, std::string> m_options;
    std::map<std::string, std::function<void(const std::string &)> > m_callbacks;
    std::set<std::vector<std::string> > m_required;
    std::map<std::string, std::string> m_help_texts;
};
