#include <GridData.h>

#include <spdlog/spdlog.h>

#define HEADER_SIZE 1

void GridData::resize(size_t width, size_t height) {
    m_width = width;
    m_height = height;
    m_data.resize(width * height * 3 + HEADER_SIZE, 0);
    m_data[0] = HEADER_BYTE;
}

Rgb GridData::get(size_t x, size_t y) const {
    auto data = &m_data[HEADER_SIZE + y * m_width * 3 + x * 3];
    return {*data, *(data + 1), *(data + 2)};
}
uint8_t* GridData::get_raw(size_t x, size_t y) {
    return &m_data[HEADER_SIZE + y * m_width * 3 + x * 3];
}

void GridData::set(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b) {
    auto data = &m_data[HEADER_SIZE + y * m_width * 3 + x * 3];
    *data = r;
    *(data + 1) = g;
    *(data + 2) = b;
}

void GridData::set(size_t x, size_t y, const Rgb& rgb) {
    set(x, y, rgb.r, rgb.g, rgb.b);
}

std::vector<uint8_t>& GridData::vector() {
    return m_data;
}

void GridData::compile() {
    for (size_t x = 0; x < m_width; ++x) {
        for (size_t y = 0; y < m_height; ++y) {
            auto cur = get_raw(x, y);
            spdlog::debug("({}, {}): ({}, {}, {})", x, y, *cur, *(cur+1), *(cur+2));
        }
    }
}