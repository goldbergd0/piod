#pragma once

#include <iostream>
#include <string>
#include <vector>

struct libusb_context;
struct libusb_device;
struct libusb_device_handle;
class Usb {
public:
    Usb() = default;
    Usb(uint64_t m_vendor_id, uint64_t m_product_id);
    virtual ~Usb();
    Usb(Usb&& other) noexcept;

    // 6. Move Assignment Operator
    Usb& operator=(Usb&& other) noexcept;

public:
    int open();
    int write_and_reopen(std::vector<uint8_t>& data, int timeout_ms = 1000, bool is_retry = false);
    bool is_open();
    int close();

private:
    Usb(const Usb& other) = delete;
    Usb& operator=(const Usb& other) = delete;


private:
    uint64_t m_vendor_id = 0x16c0;
    uint64_t m_product_id = 0x0483;
    libusb_context* m_ctx = nullptr;
    libusb_device** m_list = nullptr;
    libusb_device_handle* m_dev_handle = nullptr;
    unsigned char m_out_endpoint_address = 0;
};
