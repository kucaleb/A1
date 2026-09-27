// extract_ipv4.cpp
// Reads lines of text and extracts a single valid IPv4 address (with an
// optional :port) embedded anywhere in each line. All parsing is done by hand,
// character by character — no numeric-conversion, address-parsing, or regex
// library functions are used.

#include <cctype>
#include <iostream>
#include <string>
#include <fstream> // added by Caleb
#include <cstdlib> // added by Caleb

namespace {

// Characters that may belong to a candidate token.
bool isTokenChar(char c) {
    return std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == ':';
}

// Parses an unsigned decimal field of s[begin, end) by hand.
// Requirements: 1..maxDigits digits, no leading zero unless the field is
// exactly "0", value <= maxValue. Returns true and sets outValue on success.
bool parseField(const std::string& s, size_t begin, size_t end,
                size_t maxDigits, unsigned long maxValue,
                unsigned long& outValue) {
    size_t len = end - begin;
    if (len == 0 || len > maxDigits) return false;
    if (s[begin] == '0' && len > 1) return false;  // disallowed leading zero

    unsigned long value = 0;
    for (size_t i = begin; i < end; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
        value = value * 10 + static_cast<unsigned long>(s[i] - '0');
    }
    if (value > maxValue) return false;

    outValue = value;
    return true;
}

// Validates one complete token s[begin, end) against the grammar
//   octet '.' octet '.' octet '.' octet [ ':' port ]
// The whole token must match; no partial matches are accepted.
bool parseToken(const std::string& s, size_t begin, size_t end,
                unsigned long& outAddress, int& outPort) {
    unsigned long address = 0;
    size_t pos = begin;

    // Four octets separated by exactly three periods.
    for (int octet = 0; octet < 4; ++octet) {
        size_t fieldStart = pos;
        while (pos < end && std::isdigit(static_cast<unsigned char>(s[pos]))) ++pos;

        unsigned long value;
        if (!parseField(s, fieldStart, pos, 3, 255, value)) return false;
        address = (address << 8) | value;

        if (octet < 3) {
            if (pos >= end || s[pos] != '.') return false;  // missing separator
            ++pos;                                          // consume '.'
        }
    }

    // Optional port: a colon must be immediately after the fourth octet,
    // and the remainder of the token must be a valid port (digits only).
    int port = -1;
    if (pos < end) {
        if (s[pos] != ':') return false;  // e.g. a fifth period
        ++pos;                            // consume ':'

        unsigned long value;
        // parseField rejects any non-digit, so a second colon or a stray
        // period after the port fails here.
        if (!parseField(s, pos, end, 5, 65535, value)) return false;
        port = static_cast<int>(value);
    }

    outAddress = address;
    outPort = port;
    return true;
}

}  // namespace

// Returns true if a valid address was found, false otherwise.
// On success: outAddress holds the 32-bit value,
// and outPort holds the port number, or -1 if no port was present.
// On failure: outAddress is set to 0 and outPort is set to -1.
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    outAddress = 0;
    outPort = -1;

    size_t i = 0;
    const size_t n = str.size();
    while (i < n) {
        // Skip garbage.
        if (!isTokenChar(str[i])) {
            ++i;
            continue;
        }

        // Take the maximal run of token characters as one candidate.
        size_t start = i;
        while (i < n && isTokenChar(str[i])) ++i;

        unsigned long address;
        int port;
        if (parseToken(str, start, i, address, port)) {
            outAddress = address;
            outPort = port;
            return true;  // only one address is extracted per line
        }
    }
    return false;
}

// tells the user how to use this program and kills the program
// the entire function is written by Caleb with no ai assistence
void usage_and_die(std::ostream& o) {
    o << "invalid usage of this file" << std::endl;
    o << "either have no arguments and enter text through the command line or have an input file and output file" << std::endl;
    o << "example usage: ./extract_ipv4.exe input.txt output.txt" << std::endl;
    o << "example usage: ./extract_ipv4.exe" << std::endl;
    exit(1);
}

int main(int argc, char* argv[]) { // Caleb added argc and argv support for testing
    std::istream* input = &std::cin; // general input for either cin or a file
    std::ostream* output = &std::cout; // general output for either cout or a file
    std::ifstream i_file;
    std::ofstream o_file;
    if (argc != 1) {
        if (argc != 3) {
            usage_and_die(std::cout);
        } else {
            i_file.open(argv[1]);
            if(i_file) {
                input = &i_file;
            } else {
                usage_and_die(std::cout);
            }

            o_file.open(argv[2]);
            if (o_file) {
                output = &o_file;
            } else {
                usage_and_die(std::cout);
            }
        }
    }

    std::string line;
    while (true) {
        *output << "Enter a string (or 'END' to quit): ";
        if (!std::getline(*input, line)) break;  // treat EOF like END
        if (line == "END") break;

        unsigned long address;
        int port;
        if (extractIPv4(line, address, port)) {
            *output << "Extracted IPv4 address: "
                      << ((address >> 24) & 0xFF) << '.'
                      << ((address >> 16) & 0xFF) << '.'
                      << ((address >> 8) & 0xFF) << '.'
                      << (address & 0xFF)
                      << " (decimal value: " << address << ", port: ";
            if (port == -1)
                *output << "none";
            else
                *output << port;
            *output << ")\n";
        } else {
            *output << "No valid IPv4 address found.\n";
        }
    }
    *output << "Program terminated.\n";
    return 0;
}
