#include <iostream>
#include <string>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <libusb-1.0/libusb.h>
#include <cstdio>

#define MY_VENDOR_ID 0x16c0
#define MY_PRODUCT_ID 0x0483

int raw_example() {
    // Open the serial port
    int serial_port = open("/dev/ttyUSB0", O_RDWR); // Replace with your port

    // Check for errors
    if (serial_port < 0) {
        std::cerr << "Error " << errno << " from open: " << strerror(errno) << std::endl;
        return 1;
    }

    // Configure the serial port
    struct termios tty;
    if (tcgetattr(serial_port, &tty) != 0) {
        std::cerr << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
        return 1;
    }
    // // Set input and output baud rate to 115200
    cfsetospeed(&tty, B115200); 
    cfsetispeed(&tty, B115200);

    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CSIZE;  // Clear current size
    tty.c_cflag |= CS8;     // 8 data bits
    tty.c_cflag &= ~CRTSCTS; // Disable hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control lines

    tty.c_lflag &= ~ICANON; // Disable canonical mode (raw input)
    tty.c_lflag &= ~ECHO;   // Disable echo
    tty.c_lflag &= ~ECHOE;  // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 5; // Wait for up to 0.5s (5 * 100ms) for data
    tty.c_cc[VMIN] = 0;  // Don't block if less than VMIN bytes are available

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        std::cerr << "Error " << errno << " from tcsetattr: " << strerror(errno) << std::endl;
        return 1;
    }

    // Write data
    std::string data = "Hello from C++!\n";
    ssize_t bytes_written = write(serial_port, data.c_str(), data.length());

    if (bytes_written < 0) {
        std::cerr << "Error " << errno << " from write: " << strerror(errno) << std::endl;
        return 1;
    } else {
        std::cout << "Sent " << bytes_written << " bytes: " << data;
    }

    // Close the serial port
    close(serial_port);

    return 0;
};


libusb_context* ctx = nullptr;
libusb_device** list = nullptr;

libusb_device_handle* dev_handle = nullptr;
unsigned char out_endpoint_address = 0;

int open_usb() {
    // https://libusb.sourceforge.io/api-1.0/
    // https://stackoverflow.com/questions/17239565/how-to-most-properly-use-libusb-to-talk-to-connected-usb-devices
    // libusb_device_handle* dev_handle = nullptr;
    int r = 0; // For return values

    // Initialize libusb
    r = libusb_init(&ctx);
    if (r < 0) {
        std::cerr << "Error initializing libusb: " << libusb_error_name(r) << std::endl;
        return 1;
    }

    // Set verbosity level (optional)
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);

    // Get a list of all connected USB devices
    ssize_t count = libusb_get_device_list(ctx, &list);
    if (count < 0) {
        std::cerr << "Error getting device list: " << libusb_error_name(count) << std::endl;
        libusb_exit(ctx);
        return 1;
    }

    // Iterate through devices to find the desired one
    for (ssize_t i = 0; i < count; ++i) {
        libusb_device* device = list[i];
        libusb_device_descriptor desc;
        r = libusb_get_device_descriptor(device, &desc);
        if (r < 0) {
            std::cerr << "Error getting device descriptor: " << libusb_error_name(r) << std::endl;
            continue;
        }

        if (desc.idVendor == MY_VENDOR_ID && desc.idProduct == MY_PRODUCT_ID) {
            std::cout << "Found desired device (Vendor ID: 0x" << std::hex << desc.idVendor
                      << ", Product ID: 0x" << desc.idProduct << ")" << std::dec << std::endl;

            // Open the device
            r = libusb_open(device, &dev_handle);
            if (r < 0) {
                std::cerr << "Error opening device: " << libusb_error_name(r) << std::endl;
                dev_handle = nullptr; // Ensure dev_handle is null on error
                break;
            }

            // Detach kernel driver if active (necessary on some systems)
            if (libusb_kernel_driver_active(dev_handle, 0) == 1) { // Assuming interface 0
                r = libusb_detach_kernel_driver(dev_handle, 0);
                if (r < 0) {
                    std::cerr << "Error detaching kernel driver: " << libusb_error_name(r) << std::endl;
                    libusb_close(dev_handle);
                    dev_handle = nullptr;
                    break;
                }
            }

            // Claim the interface (assuming interface 0)
            r = libusb_claim_interface(dev_handle, 0);
            if (r < 0) {
                std::cerr << "Error claiming interface: " << libusb_error_name(r) << std::endl;
                libusb_close(dev_handle);
                dev_handle = nullptr;
                break;
            }

            // Find an OUT endpoint
            libusb_config_descriptor* config_desc = nullptr;
            r = libusb_get_active_config_descriptor(device, &config_desc);
            if (r < 0) {
                std::cerr << "Error getting active config descriptor: " << libusb_error_name(r) << std::endl;
                libusb_release_interface(dev_handle, 0);
                libusb_close(dev_handle);
                dev_handle = nullptr;
                break;
            }

            // unsigned char out_endpoint_address = 0;
            for (int cfg_idx = 0; cfg_idx < config_desc->bNumInterfaces; ++cfg_idx) {
                const libusb_interface& interface = config_desc->interface[cfg_idx];
                for (int alt_idx = 0; alt_idx < interface.num_altsetting; ++alt_idx) {
                    const libusb_interface_descriptor& if_desc = interface.altsetting[alt_idx];
                    for (int ep_idx = 0; ep_idx < if_desc.bNumEndpoints; ++ep_idx) {
                        const libusb_endpoint_descriptor& ep_desc = if_desc.endpoint[ep_idx];
                        if ((ep_desc.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
                            out_endpoint_address = ep_desc.bEndpointAddress;
                            std::cout << "Found OUT endpoint with address: 0x" << std::hex
                                      << (int)out_endpoint_address << std::dec << std::endl;
                            break; // Found an OUT endpoint, exit inner loops
                        }
                    }
                    if (out_endpoint_address != 0) break;
                }
                if (out_endpoint_address != 0) break;
            }
            libusb_free_config_descriptor(config_desc);

            if (out_endpoint_address == 0) {
                std::cerr << "No OUT endpoint found for the device." << std::endl;
                libusb_release_interface(dev_handle, 0);
                libusb_close(dev_handle);
                dev_handle = nullptr;
                break;
            }
            return 0; // Exit device search loop after finding and interacting with the device
        }
    }

    return 0;
};

void cleanup_usb() {
    // Release interface and close device
    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    dev_handle = nullptr; // Clear handle after closing
    // Free the device list and exit libusb
    libusb_free_device_list(list, 1); // Free devices in list as well
    libusb_exit(ctx);
}

void write_usb(std::vector<uint8_t>& data) {
    // Write data to the OUT endpoint
    // std::vector<uint8_t> data{100, 0, 0};
    int actual_length;
    int timeout_ms = 1000; // 1 second timeout

    // libusb_device_handle *dev_handle,
    // unsigned char endpoint, unsigned char *data, int length,
    // int *actual_length, unsigned int timeout
    std::cout << "Writing to endpoint 0x" << std::hex << (int)out_endpoint_address << std::dec << std::endl;
    std::cout << "Data: ";
    for (const auto& byte : data) {
        std::cout << (int)byte << " ";
    }
    std::cout << std::endl;
    int r = libusb_bulk_transfer(dev_handle, out_endpoint_address,
                                data.data(), data.size(),
                                &actual_length, timeout_ms);

    if (r == 0) {
        std::cout << "Successfully wrote " << actual_length << " bytes to the device." << std::endl;
    } else {
        std::cerr << "Error writing data: " << libusb_error_name(r) << std::endl;
    }
    // r = libusb_bulk_transfer(dev_handle, out_endpoint_address, NULL, 0, &actual_length, timeout_ms);
    // if (r == 0) {
    //     printf("Zero-length packet sent successfully. Transferred: %d bytes\n", actual_length);
    // } else {
    //     fprintf(stderr, "Error sending zero-length packet: %s\n", libusb_error_name(r));
    // }
}