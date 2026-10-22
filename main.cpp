#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

int main() {
    // Open the file in binary mode and immediately jump to the end ("ate")
    std::string filename = "pokemon_red.gb";
    std::ifstream rom_file(filename, std::ios::binary | std::ios::ate);

    if (!rom_file.is_open()) {
        std::cerr << "Error: Could not open " << filename << ". Check the path!\n";
        return 1;
    }

    // Get the ROM size (since we opened it at the end, tellg() gives the total size)
    std::streamsize rom_size = rom_file.tellg();
    
    // Move the file pointer back to the beginning to start reading
    rom_file.seekg(0, std::ios::beg);

    std::cout << "ROM successfully loaded!\n";
    std::cout << "Size: " << rom_size << " bytes (" 
              << std::fixed << std::setprecision(2) 
              << static_cast<double>(rom_size) / (1024 * 1024) << " MB)\n";

    // Allocate a dynamic vector buffer to hold the raw bytes
    // uint8_t is an unsigned 8-bit integer (0 to 255), perfect for raw bytes.
    std::vector<uint8_t> rom_buffer(rom_size);

    // Read the entire ROM into our vector buffer
    if (!rom_file.read(reinterpret_cast<char*>(rom_buffer.data()), rom_size)) {
        std::cerr << "Error occurred while reading the ROM data.\n";
        return 1;
    }
    
    // Close the file stream explicitly (though it closes automatically when out of scope)
    rom_file.close();

    return 0;
}
