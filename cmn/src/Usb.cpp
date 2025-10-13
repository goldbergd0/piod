#include <Usb.h>

#include <iostream>
#include <string>
#include <string_view>

#include <libusb-1.0/libusb.h>
#include "spdlog/spdlog.h"
 
#include <fmt/core.h>

template <> struct fmt::formatter<libusb_device_descriptor> {
    // A simple formatter that doesn't parse any format specs.
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const libusb_device_descriptor& desc, fmt::format_context& ctx) const {
        return fmt::format_to(
            ctx.out(),
            "libusb_device_descriptor(bcdUSB={:#06x}, bDeviceClass={:#04x}, "
            "idVendor={:#06x}, idProduct={:#06x}, bcdDevice={:#06x})",
            desc.bcdUSB, desc.bDeviceClass, desc.idVendor, desc.idProduct, desc.bcdDevice
        );
    }
};

Usb::Usb(uint64_t m_vendor_id, uint64_t m_product_id)
    : m_vendor_id(m_vendor_id), m_product_id(m_product_id) {}

Usb::~Usb() {
    close();
}

Usb::Usb(Usb&& other) noexcept {
    if (this != &other) {
        m_vendor_id = other.m_vendor_id;
        m_product_id = other.m_product_id;
        m_ctx = other.m_ctx;
        m_list = other.m_list;
        m_dev_handle = other.m_dev_handle;
        m_out_endpoint_address = other.m_out_endpoint_address;

        // Nullify the other's pointers to prevent double free
        other.m_ctx = nullptr;
        other.m_list = nullptr;
        other.m_dev_handle = nullptr;
        other.m_out_endpoint_address = 0;
    }
}

Usb& Usb::operator=(Usb&& other) noexcept {
    *this = std::move(other);
    return *this;
}

int Usb::open() {
    // https://libusb.sourceforge.io/api-1.0/
    // https://stackoverflow.com/questions/17239565/how-to-most-properly-use-libusb-to-talk-to-connected-usb-devices
    // libusb_device_handle* dev_handle = nullptr;
    int r = 0; // For return values

    // Initialize libusb
    r = libusb_init(&m_ctx);
    if (r < 0) {
        spdlog::error("Error initializing libusb: {}",  libusb_error_name(r));
        return r;
    }

    // Set verbosity level (optional)
    
    if (spdlog::get_level() == spdlog::level::debug) 
        libusb_set_option(m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
    else if (spdlog::get_level() == spdlog::level::info)
        libusb_set_option(m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
    else if (spdlog::get_level() == spdlog::level::warn)
        libusb_set_option(m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);
    else if (spdlog::get_level() == spdlog::level::err)
        libusb_set_option(m_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_ERROR);

    // Get a list of all connected USB devices
    ssize_t count = libusb_get_device_list(m_ctx, &m_list);
    if (count < 0) {
        spdlog::error("Error getting device list: {}", libusb_error_name(count));
        close();
        return count;
    }

    // Iterate through devices to find the desired one
    for (ssize_t i = 0; i < count; ++i) {
        libusb_device* device = m_list[i];
        libusb_device_descriptor desc;
        r = libusb_get_device_descriptor(device, &desc);
        if (r < 0) {
            spdlog::warn("Error getting device descriptor: {}",  libusb_error_name(r));
            continue;
        }
        spdlog::info("Found usb device: {}", desc);

        // if (desc.idVendor == m_vendor_id && desc.idProduct == m_product_id) {
        //     std::cout << "Found desired device (Vendor ID: 0x" << std::hex << desc.idVendor
        //               << ", Product ID: 0x" << desc.idProduct << ")" << std::dec << std::endl;

        // Open the device
        r = libusb_open(device, &m_dev_handle);
        if (r < 0) {
            spdlog::error("Error opening device: {}", libusb_error_name(r));
            m_dev_handle = nullptr; // Ensure dev_handle is null on error
            continue;
        }
        std::string product_string;
        product_string.resize(256);
        libusb_get_string_descriptor_ascii(m_dev_handle, desc.iProduct, reinterpret_cast<unsigned char *>(product_string.data()), product_string.size());
        // [2025-10-13 17:16:15.308] [info] Found usb device: libusb_device_descriptor(bcdUSB=0x0200, bDeviceClass=0xef, idVendor=0x16c0, idProduct=0x0483, bcdDevice=0x0279)
        // [2025-10-13 17:16:15.308] [info] Opened device (0x16c0/0x0483): USB Serial
        spdlog::info("Opened device (0x{:04x}/0x{:04x}): {}", desc.idVendor, desc.idProduct, product_string);
        if (desc.idVendor != m_vendor_id || desc.idProduct != m_product_id) {
            spdlog::info("Device does not match desired VID/PID (0x{:04x}/0x{:04x}), closing.", m_vendor_id, m_product_id);
            libusb_close(m_dev_handle);
            m_dev_handle = nullptr;
            continue;
        }
        // Detach kernel driver if active (necessary on some systems)
        if (libusb_kernel_driver_active(m_dev_handle, 0) == 1) { // Assuming interface 0
            r = libusb_detach_kernel_driver(m_dev_handle, 0);
            if (r < 0) {
                spdlog::critical("Error detaching kernel driver: {}", libusb_error_name(r));
                close();
                return r;
            }
        }

        // Claim the interface (assuming interface 0)
        r = libusb_claim_interface(m_dev_handle, 0);
        if (r < 0) {
            spdlog::critical("Error claiming interface: {}", libusb_error_name(r));
            close();
            return r;
        }

        // Find an OUT endpoint
        libusb_config_descriptor* config_desc = nullptr;
        r = libusb_get_active_config_descriptor(device, &config_desc);
        if (r < 0) {
            spdlog::critical("Error getting active config descriptor: {}", libusb_error_name(r));
            close();
            return r;
        }

        // unsigned char out_endpoint_address = 0;
        for (int cfg_idx = 0; cfg_idx < config_desc->bNumInterfaces && m_out_endpoint_address == 0; ++cfg_idx) {
            const libusb_interface& interface = config_desc->interface[cfg_idx];
            for (int alt_idx = 0; alt_idx < interface.num_altsetting && m_out_endpoint_address == 0; ++alt_idx) {
                const libusb_interface_descriptor& if_desc = interface.altsetting[alt_idx];
                for (int ep_idx = 0; ep_idx < if_desc.bNumEndpoints && m_out_endpoint_address == 0; ++ep_idx) {
                    const libusb_endpoint_descriptor& ep_desc = if_desc.endpoint[ep_idx];
                    if ((ep_desc.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
                        m_out_endpoint_address = ep_desc.bEndpointAddress;
                        spdlog::info("Found OUT endpoint with address: {:#06x}", (int)m_out_endpoint_address);
                    }
                }
            }
        }
        libusb_free_config_descriptor(config_desc);

        if (m_out_endpoint_address == 0) {
            spdlog::critical("No OUT endpoint found for the device.");
            close();
            return -1;
        }
        return 0; // Exit device search loop after finding and interacting with the device
    }

    spdlog::error("No USB devices found.");
    return -1;
};

int Usb::close() {
    // Release interface and close device
    if (m_dev_handle) {
        libusb_release_interface(m_dev_handle, 0);
        libusb_close(m_dev_handle);
    }
    // Free the device list and exit libusb
    if (m_list) libusb_free_device_list(m_list, 1);
    if (m_ctx) libusb_exit(m_ctx);
    
    m_dev_handle = nullptr;
    m_list = nullptr;
    m_ctx = nullptr;
    return -1;
}

bool Usb::is_open() {
    return m_dev_handle;
}

int Usb::write_and_reopen(std::vector<uint8_t>& data, int timeout_ms, bool is_retry) {
    if (!is_open()) {
        spdlog::error("Device not open. Cannot write data.");
        return -1;
    }
    // Write data to the OUT endpoint
    int actual_length;
    // CANT SEND DATA SMALLER THAN 7 BYTES
    if (data.size() < 8) {
        spdlog::warn("Data size {} is less than 8 bytes, padding with zeros.", data.size());
        data.resize(8, 0);
    }

    int r = libusb_bulk_transfer(m_dev_handle, m_out_endpoint_address,
                                data.data(), data.size(),
                                &actual_length, timeout_ms);

    if (r == 0) {
        spdlog::debug("Successfully wrote {} bytes to the device.", actual_length);
    } else {
        spdlog::error("Error writing data: {} ({}) - trying to reopen", libusb_error_name(r), r);
        if (!is_retry) {
            r = open();
            r = write_and_reopen(data, timeout_ms, true);
        }
    }
    return r;
}