#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

class GameBoy{
private:
    //Memory
    std::vector<uint8_t> rom_buffer;
    std::vector<uint8_t> wram_buffer;

    //CPU
    bool IME = 1;

    //Registers
    uint16_t AF = 0x01B0; //lower byte of this register has the flags 7 = Zero Flag, 6 = Substraction Flag, 5 = HC Flag, 4 = Carry Flag
    uint16_t BC = 0x0013;
    uint16_t DE = 0x00D8;
    uint16_t HL = 0x014D;
    uint16_t SP = 0xFFFE;
    uint16_t PC = 0x0100;

    uint8_t get_A(){ return (AF & 0xFF00) >> 8; }
    void set_A(uint8_t value){  AF = (value << 8) | (AF & 0x00FF);   }

    uint8_t get_B(){ return (BC & 0xFF00) >> 8; }
    void set_B(uint8_t value){  BC = (value << 8) | (BC & 0x00FF);   }

    uint8_t get_C(){ return (BC & 0x00FF); }
    void set_C(uint8_t value){ BC = (BC & 0xFF00) | (value); }

    uint8_t get_D(){ return (DE & 0xFF00) >> 8; }
    void set_D(uint8_t value){  DE = (value << 8) | (DE & 0x00FF);   }

    uint8_t get_E(){ return (DE & 0x00FF); }
    void set_E(uint8_t value){ DE = (DE & 0xFF00) | (value); }

    uint8_t get_L(){ return (HL & 0x00FF); }
    void set_L(uint8_t value){ HL = (HL & 0xFF00) | (value); }

    uint8_t get_H(){ return (HL & 0xFF00) >> 8; }
    void set_H(uint8_t value){  HL = (value << 8) | (HL & 0x00FF);   }

    //Flags
    bool get_Z_flag() { return (AF & (1 << 7)) != 0; }
    void set_Z_flag(bool value) {
        if (value) {
            AF |= (1 << 7);
        } else {
            AF &= ~(1 << 7);
        }
    }

    bool get_N_flag() { return (AF & (1 << 6)) != 0; }
    void set_N_flag(bool value) {
        if (value) {
            AF |= (1 << 6);
        } else {
            AF &= ~(1 << 6);
        }
    }

    bool get_H_flag() { return (AF & (1 << 5)) != 0; }
    void set_H_flag(bool value) {
        if (value) {
            AF |= (1 << 5);
        } else {
            AF &= ~(1 << 5);
        }
    }

    bool get_C_flag() { return (AF & (1 << 4)) != 0; }
    void set_C_flag(bool value) {
        if (value) {
            AF |= (1 << 4);
        } else {
            AF &= ~(1 << 4);
        }
    }

public:
    uint8_t read_byte(uint16_t address) {
        if (address < 0x8000) {
            return rom_buffer[address];
        }
        else if (address >= 0xC000 && address <= 0xDFFF) {
            return wram_buffer[address - 0xC000];
        }
        return 0;
    }

    void write_byte(uint16_t address, uint8_t value) {
        if (address >= 0xC000 && address <= 0xDFFF) {
            wram_buffer[address - 0xC000] = value;
        }
    }

    int run() {
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


        // Read the entire ROM into our vector buffer
        if (!rom_file.read(reinterpret_cast<char*>(rom_buffer.data()), rom_size)) {
            std::cerr << "Error occurred while reading the ROM data.\n";
            return 1;
        }

        //To print the desired address
        std::cout << "0x" 
        << std::hex                       // Switch stream to hexadecimal mode
        << std::setw(2)                   // Force the output to be 2 characters wide
        << std::setfill('0')              // Pad with a '0' if it's a single digit (e.g., 0x05 instead of 0x5)
        << static_cast<int>(rom_buffer[0x0100]) 
        << std::endl;

        //Instruction execution
        while(true){
            std::cout << "PC is now " << PC << std::endl;
            uint8_t opcode = read_byte(PC); 
            switch(opcode){
                //NOP
                case 0x00: {
                    PC++;
                    break;
                }
                //STOP
                case 0x10: {
                    PC++;
                    PC++;
                    std::cout << "Entering low power mode by STOP instruction. Next byte is ignored" << std::endl;
                    break;
                }
                //JR NZ, e8
                case 0X20: {
                    int8_t offset = static_cast<int8_t>(rom_buffer[PC + 1]);
                    PC += 2;
                    if (!get_Z_flag()){
                        PC += offset;
                    }
                    break;
                }
                case 0x30: {
                    int8_t offset = static_cast<int8_t>(rom_buffer[PC + 1]);
                    PC += 2;
                    if (!get_C_flag()){
                        PC += offset;
                    }
                    break;
                }
                case 0x40: {
                    PC++;
                    set_B(get_B());
                    break;
                }
                case 0x50:{
                    PC++;
                    set_D(get_B());
                    break;
                }
                case 0x60:{
                    PC++;
                    set_H(get_B());
                    break;
                }
                case 0x70: {
                    PC++;
                    write_byte(HL, get_B());
                    break;
                }
                case 0x80: {
                    uint8_t A = get_A();
                    uint8_t B = get_B();
                    uint8_t result = A + B;
                    
                    
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    if ((A & 0x0F) + (B & 0x0F) > 0x0F) { set_H_flag(1); }
                    else {set_H_flag(0);}
                    set_C_flag((result < A)); 

                    set_A(result);
                    PC++;
                    break;
                }
                case 0x90: {
                    uint8_t A = get_A();
                    uint8_t B = get_B();
                    uint8_t result = A - B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(1);
                    set_C_flag((B > A)); 
                    if ((A & 0x0F) < (B & 0x0F)) { set_H_flag(1); }
                    else {set_H_flag(0);}

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xA0: {
                    uint8_t A = get_A();
                    uint8_t B = get_B();
                    uint8_t result = A & B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(1);
                    set_C_flag(0);

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xB0: {
                    uint8_t A = get_A();
                    uint8_t B = get_B();
                    uint8_t result = A | B;
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0); 

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xC0:
                    if (!get_Z_flag()){ 
                        uint8_t low = read_byte(SP);
                        SP++;
                        uint8_t high = read_byte(SP);
                        SP++;

                        PC = (high << 8) | low;
                    }
                    else {PC++;}
                    break;
                case 0xD0:
                    if(!get_C_flag()){
                        uint8_t low = read_byte(SP);
                        SP++;
                        uint8_t high = read_byte(SP);
                        SP++;

                        PC = (high << 8) | low;
                    }
                    else {PC++;}
                    break;
                case 0xE0:
                    uint8_t address = rom_buffer[PC+1];
                    uint16_t t_address = 0xFF00 + address;
                    write_byte(t_address, get_A());
                    PC += 2;
                    break;
                case 0xF0:
                    uint8_t address = rom_buffer[PC+1];
                    uint16_t t_address = 0xFF00 + address;
                    set_A(t_address);
                    PC += 2;
                    break;
                case 0x01:{
                    uint8_t B = rom_buffer[PC+1];
                    uint8_t A = rom_buffer[PC+2];
                    BC = (A << 8) + B;
                    PC += 3;
                    break;
                }
                case 0x11:{
                    uint8_t B = rom_buffer[PC+1];
                    uint8_t A = rom_buffer[PC+2];
                    DE = (A << 8) + B;
                    PC += 3;
                    break;
                }
                case 0x21: {
                    uint8_t B = rom_buffer[PC+1];
                    uint8_t A = rom_buffer[PC+2];
                    HL = (A << 8) + B;
                    PC += 3;
                    break;
                }
                case 0x31: {
                    uint8_t B = rom_buffer[PC+1];
                    uint8_t A = rom_buffer[PC+2];
                    SP = (A << 8) + B;
                    PC += 3;
                    break;
                }
                case 0x41: {
                    set_B(get_C());
                    PC++;
                    break;
                }
                case 0x51: {
                    set_D(get_C());
                    PC++;
                    break;
                }
                case 0x61: {
                    set_H(get_C());
                    PC++;
                    break;
                }
                case 0x71: {
                    write_byte(HL, get_C());
                    PC++;
                    break;
                }
                case 0x81: {
                    uint8_t A = get_A();
                    uint8_t B = get_C();
                    uint8_t result = A + B;
                    
                    
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    if ((A & 0x0F) + (B & 0x0F) > 0x0F) { set_H_flag(1); }
                    else {set_H_flag(0);}
                    set_C_flag((result < A)); 

                    set_A(result);
                    PC++;
                    break;
                }
                case 0x91: {
                    uint8_t A = get_A();
                    uint8_t B = get_C();
                    uint8_t result = A - B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(1);
                    set_C_flag((B > A)); 
                    if ((A & 0x0F) < (B & 0x0F)) { set_H_flag(1); }
                    else {set_H_flag(0);}

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xA1: {
                    uint8_t A = get_A();
                    uint8_t B = get_C();
                    uint8_t result = A & B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(1);
                    set_C_flag(0);

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xB1: {
                    uint8_t A = get_A();
                    uint8_t B = get_C();
                    uint8_t result = A | B;
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0); 

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xC1:
                    uint8_t low = read_byte(SP);
                    SP++;
                    uint8_t high = read_byte(SP);
                    SP++;

                    BC = (high << 8) + low;
                    PC++;
                    break;
                case 0xD1:
                    uint8_t low = read_byte(SP);
                    SP++;
                    uint8_t high = read_byte(SP);
                    SP++;

                    DE = (high << 8) + low;
                    PC++;
                    break;
                case 0xE1:
                    uint8_t low = read_byte(SP);
                    SP++;
                    uint8_t high = read_byte(SP);
                    SP++;

                    HL = (high << 8) + low;
                    PC++;
                    break;
                case 0xF1:
                    uint8_t low = read_byte(SP);
                    SP++;
                    uint8_t high = read_byte(SP);
                    SP++;

                    AF = (high << 8) + low;
                    PC++;
                    break;
                case 0x02:{
                    write_byte(BC, get_A());
                    PC++;
                    break;
                }
                case 0x12:{
                    write_byte(DE, get_A());
                    PC++;
                    break;
                }
                case 0x22: {
                    write_byte(HL, get_A());
                    HL++;
                    PC++;
                    break;
                }
                case 0x32: {
                    write_byte(HL, get_A());
                    HL--;
                    PC++;
                    break;
                }
                case 0x42: {
                    set_B(get_D());
                    PC++;
                    break;
                }
                case 0x52: {
                    set_D(get_D());
                    PC++;
                    break;
                }
                case 0x62: {
                    set_H(get_D());
                    PC++;
                    break;
                }
                case 0x72: {
                    write_byte(HL, get_D());
                    PC++;
                    break;
                }
                case 0x82: {
                    uint8_t A = get_A();
                    uint8_t B = get_D();
                    uint8_t result = A + B;
                    
                    
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    if ((A & 0x0F) + (B & 0x0F) > 0x0F) { set_H_flag(1); }
                    else {set_H_flag(0);}
                    set_C_flag((result < A)); 

                    set_A(result);
                    PC++;
                    break;
                }
                case 0x92: {
                    uint8_t A = get_A();
                    uint8_t B = get_D();
                    uint8_t result = A - B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(1);
                    set_C_flag((B > A)); 
                    if ((A & 0x0F) < (B & 0x0F)) { set_H_flag(1); }
                    else {set_H_flag(0);}

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xA2: {
                    uint8_t A = get_A();
                    uint8_t B = get_D();
                    uint8_t result = A & B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(1);
                    set_C_flag(0);

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xB2: {
                    uint8_t A = get_A();
                    uint8_t B = get_D();
                    uint8_t result = A | B;
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0); 

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xC2:
                    if (!get_Z_flag()){
                        uint8_t low = read_byte(PC+1);
                        uint8_t high = read_byte(PC+2);
                        PC = (high << 8) + low;
                    }
                    else{ PC++; }
                    break;
                case 0xD2:
                    if (!get_C_flag()){
                        uint8_t low = read_byte(PC+1);
                        uint8_t high = read_byte(PC+2);
                        PC = (high << 8) + low;
                    }
                    else{ PC++; }
                    break;
                case 0xE2:
                    write_byte(get_C() + 0xFF00, get_A());
                    PC++;
                    break;
                case 0xF2:
                    write_byte(get_A() + 0xFF00, read_byte(get_C() + 0xFF00));
                    PC++;
                    break;
                case 0x03:
                    BC++;
                    PC++;
                    break;
                case 0x13:
                    DE++;
                    PC++;
                    break;
                case 0x23:
                    HL++;
                    PC++;
                    break;
                case 0x33:
                    SP++;
                    PC++;
                    break;
                case 0x43: {
                    set_B(get_E());
                    PC++;
                    break;
                }
                case 0x53: {
                    set_D(get_E());
                    PC++;
                    break;
                }
                case 0x63: {
                    set_H(get_E());
                    PC++;
                    break;
                }
                case 0x73: {
                    write_byte(HL, get_E());
                    PC++;
                    break;
                }
                case 0x83: {
                    uint8_t A = get_A();
                    uint8_t B = get_E();
                    uint8_t result = A + B;
                    
                    
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    if ((A & 0x0F) + (B & 0x0F) > 0x0F) { set_H_flag(1); }
                    else {set_H_flag(0);}
                    set_C_flag((result < A)); 

                    set_A(result);
                    PC++;
                    break;
                }
                case 0x93: {
                    uint8_t A = get_A();
                    uint8_t B = get_E();
                    uint8_t result = A - B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(1);
                    set_C_flag((B > A)); 
                    if ((A & 0x0F) < (B & 0x0F)) { set_H_flag(1); }
                    else {set_H_flag(0);}

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xA3: {
                    uint8_t A = get_A();
                    uint8_t B = get_E();
                    uint8_t result = A & B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(1);
                    set_C_flag(0);

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xB3: {
                    uint8_t A = get_A();
                    uint8_t B = get_E();
                    uint8_t result = A | B;
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0); 

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xC3:
                    uint8_t low = read_byte(PC+1);
                    uint8_t high = read_byte(PC+2);
                    PC = (high << 8) + low;
                    break;
                case 0xF3:
                    IME = 0;
                    break;
                case 0x04:
                    uint8_t val = get_B() + 1;
                    set_B(val);
                    set_Z_flag(val == 0);
                    set_N_flag(false);
                    set_H_flag((val & 0x0F) == 0);
                    break;
                case 0x14:
                    uint8_t val = get_D() + 1;
                    set_D(val);
                    set_Z_flag(val == 0);
                    set_N_flag(false);
                    set_H_flag((val & 0x0F) == 0);
                    break;
                case 0x24:
                    uint8_t val = get_H() + 1;
                    set_H(val);
                    set_Z_flag(val == 0);
                    set_N_flag(false);
                    set_H_flag((val & 0x0F) == 0);
                    break;
                case 0x34:
                    uint8_t val = read_byte(HL) + 1;
                    write_byte(HL, val);
                    set_Z_flag(val == 0);
                    set_N_flag(false);
                    set_H_flag((val & 0x0F) == 0);
                    break;
                case 0x44: {
                    set_B(get_H());
                    PC++;
                    break;
                }
                case 0x54: {
                    set_D(get_H());
                    PC++;
                    break;
                }
                case 0x64: {
                    set_H(get_H());
                    PC++;
                    break;
                }
                case 0x74: {
                    write_byte(HL, get_H());
                    PC++;
                    break;
                }
                case 0x84: {
                    uint8_t A = get_A();
                    uint8_t B = get_H();
                    uint8_t result = A + B;
                    
                    
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    if ((A & 0x0F) + (B & 0x0F) > 0x0F) { set_H_flag(1); }
                    else {set_H_flag(0);}
                    set_C_flag((result < A)); 

                    set_A(result);
                    PC++;
                    break;
                }
                case 0x94: {
                    uint8_t A = get_A();
                    uint8_t B = get_H();
                    uint8_t result = A - B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(1);
                    set_C_flag((B > A)); 
                    if ((A & 0x0F) < (B & 0x0F)) { set_H_flag(1); }
                    else {set_H_flag(0);}

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xA4: {
                    uint8_t A = get_A();
                    uint8_t B = get_H();
                    uint8_t result = A & B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(1);
                    set_C_flag(0);

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xB4: {
                    uint8_t A = get_A();
                    uint8_t B = get_H();
                    uint8_t result = A | B;
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0); 

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xC4:
                    if (!get_Z_flag()){
                        uint8_t low = read_byte(PC+1);
                        uint8_t high = read_byte(PC+2);

                        SP--;
                        write_byte(SP, PC+3 & 0x00FF);
                        SP--;
                        write_byte(SP, (PC+3 & 0xFF00) >> 8);

                        PC = (high << 8) + low;
                        break;
                    }
                    else { PC += 3; }
                    break;
                case 0xD4:
                    if (!get_C_flag()){
                        uint8_t low = read_byte(PC+1);
                        uint8_t high = read_byte(PC+2);

                        SP--;
                        write_byte(SP, PC+3 & 0x00FF);
                        SP--;
                        write_byte(SP, (PC+3 & 0xFF00) >> 8);

                        PC = (high << 8) + low;
                        break;
                    }
                    else { PC += 3; }
                    break;
                case 0x05:
                    uint8_t val = get_B() - 1;
                    set_B(val);
                    
                    set_Z_flag(val == 0);                 
                    set_N_flag(true);                     
                    set_H_flag((val & 0x0F) == 0x0F);
                    break;
                case 0x15:
                    uint8_t val = get_D() - 1;
                    set_D(val);
                    
                    set_Z_flag(val == 0);                 
                    set_N_flag(true);                     
                    set_H_flag((val & 0x0F) == 0x0F);
                    break;
                case 0x25:
                    uint8_t val = get_H() - 1;
                    set_H(val);
                    
                    set_Z_flag(val == 0);                 
                    set_N_flag(true);                     
                    set_H_flag((val & 0x0F) == 0x0F);
                    break;
                case 0x35:
                    uint8_t val = read_byte(HL) - 1;
                    write_byte(HL, val);
                    
                    set_Z_flag(val == 0);                 
                    set_N_flag(true);                     
                    set_H_flag((val & 0x0F) == 0x0F);
                    break;
                case 0x45: {
                    set_B(get_L());
                    PC++;
                    break;
                }
                case 0x55: {
                    set_D(get_L());
                    PC++;
                    break;
                }
                case 0x65: {
                    set_H(get_L());
                    PC++;
                    break;
                }
                case 0x75: {
                    write_byte(HL, get_L());
                    PC++;
                    break;
                }
            }
        }

        // Close the file stream explicitly (though it closes automatically when out of scope)
        rom_file.close();

        return 0;
    }
};