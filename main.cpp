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
    
    uint16_t pc = 0x0100;

    //To print the desired address
    std::cout << "0x" 
    << std::hex                       // Switch stream to hexadecimal mode
    << std::setw(2)                   // Force the output to be 2 characters wide
    << std::setfill('0')              // Pad with a '0' if it's a single digit (e.g., 0x05 instead of 0x5)
    << static_cast<int>(rom_buffer[0x0100]) 
    << std::endl;

    //Registers (8 bits)
    uint8_t A;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;

    //Registers (16 bits)
    uint16_t AF; //lower byte of this register has the flags 7 = Zero Flag, 6 = Substraction Flag, 5 = HC Flag, 4 = Carry Flag
    uint16_t BC;
    uint16_t DE;
    uint16_t HL;
    uint16_t SP;
    uint16_t PC;

    //Flags
    uint8_t Z = (AF << 8) >> 15;
    uint8_t N = (AF << 9) >> 15;
    uint8_t H = (AF << 10) >> 15;
    uint8_t C = (AF << 11) >> 15;
    
    //Instruction execution
    while(true){
        std::cout << "PC is now " << pc << std::endl;
        switch(rom_buffer[PC]){
            //NOP
            case 0x00:
                PC++;
                break;
            //STOP
            case 0x10:
                PC++;
                PC++;
                std::cout << "Entering low power mode by STOP instruction. Next byte is ignored" << std::endl;
                break;
            //JR NZ, e8
            case 0X20:
                if (Z){
                    PC++;
                    int8_t addr = 0;
                    addr = addr;
                    break;
                }
                PC++;
                break;
        }
    }

    // Close the file stream explicitly (though it closes automatically when out of scope)
    rom_file.close();

    return 0;
}
