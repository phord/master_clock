#include <stdint.h>

class IPAddress {
private:
    uint8_t _address[4];  // IPv4 address
    // Access the raw byte array containing the address.  Because this returns a pointer
    // to the internal structure rather than a copy of the address this function should only
    // be used when you know that the usage of the returned uint8_t* will be transient and not
    // stored.
    uint8_t* raw_address() { return _address; };

public:
    // Constructors
    IPAddress();
    IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet) { }
    IPAddress(uint32_t address);
    IPAddress(const uint8_t *address);
};
