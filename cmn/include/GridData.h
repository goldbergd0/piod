#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <tuple>

class GridData {
public:
    GridData() = default;
    GridData(size_t width, size_t height) : m_width(width), m_height(height) {
        resize(width, height);
    }

    void resize(size_t width, size_t height);
    
    std::tuple<uint8_t, uint8_t, uint8_t> get(size_t x, size_t y) const;
    void set(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b);
    void set(size_t x, size_t y, const std::tuple<uint8_t, uint8_t, uint8_t>& rgb);

    std::vector<uint8_t>& data() { return m_data; }

    size_t width() const { return m_width; }
    size_t height() const { return m_height; }


private:
    size_t m_width = 0;
    size_t m_height = 0;
    std::vector<uint8_t> m_data;
    uint8_t const HEADER_BYTE = 42;
};