#include <GridData.h>

#define HEADER_SIZE 1

void GridData::resize(size_t width, size_t height) {
    m_width = width;
    m_height = height;
    m_data.resize(width * height * 3 + HEADER_SIZE, 0);
    m_data[0] = HEADER_BYTE;
}

std::tuple<uint8_t, uint8_t, uint8_t> GridData::get(size_t x, size_t y) const {
    auto data = &m_data[HEADER_SIZE + y * m_width * 3 + x * 3];
    return {*data, *(data + 1), *(data + 2)};
}

void GridData::set(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b) {
    auto data = &m_data[HEADER_SIZE + y * m_width * 3 + x * 3];
    *data = r;
    *(data + 1) = g;
    *(data + 2) = b;
}

void GridData::set(size_t x, size_t y, std::tuple<uint8_t, uint8_t, uint8_t> const& rgb) {
    set(x, y, std::get<0>(rgb), std::get<1>(rgb), std::get<2>(rgb));
}
