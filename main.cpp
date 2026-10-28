#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

class GameBoy{
private:
    //Memory
    std::vector<uint8_t> rom_buffer;
    std::vector<uint8_t> wram_buffer = std::vector<uint8_t>(0x2000, 0);;

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

        rom_buffer.resize(rom_size);
        
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
            std::cout << (int)opcode << std::endl; 
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
                case 0xC0:{
                    if (!get_Z_flag()){ 
                        uint8_t low = read_byte(SP);
                        SP++;
                        uint8_t high = read_byte(SP);
                        SP++;

                        PC = (high << 8) | low;
                    }
                    else {PC++;}
                    break;
                }
                case 0xD0:{
                    if(!get_C_flag()){
                        uint8_t low = read_byte(SP);
                        SP++;
                        uint8_t high = read_byte(SP);
                        SP++;

                        PC = (high << 8) | low;
                    }
                    else {PC++;}
                    break;
                }
                case 0xE0:{
                    uint8_t address = rom_buffer[PC+1];
                    uint16_t t_address = 0xFF00 + address;
                    write_byte(t_address, get_A());
                    PC += 2;
                    break;
                }
                case 0xF0:{
                    uint8_t address = rom_buffer[PC+1];
                    uint16_t t_address = 0xFF00 + address;
                    set_A(t_address);
                    PC += 2;
                    break;
                }
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
                case 0xC1:{
                    uint8_t low = read_byte(SP);
                    SP++;
                    uint8_t high = read_byte(SP);
                    SP++;

                    BC = (high << 8) + low;
                    PC++;
                    break;
                }
                case 0xD1:{
                    uint8_t low = read_byte(SP);
                    SP++;
                    uint8_t high = read_byte(SP);
                    SP++;

                    DE = (high << 8) + low;
                    PC++;
                    break;}
                case 0xE1:{
                    uint8_t low = read_byte(SP);
                    SP++;
                    uint8_t high = read_byte(SP);
                    SP++;

                    HL = (high << 8) + low;
                    PC++;
                    break;}
                case 0xF1:{
                    uint8_t low = read_byte(SP);
                    SP++;
                    uint8_t high = read_byte(SP);
                    SP++;

                    AF = (high << 8) + low;
                    PC++;
                    break;}
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
                case 0xC2:{
                    if (!get_Z_flag()){
                        uint8_t low = read_byte(PC+1);
                        uint8_t high = read_byte(PC+2);
                        PC = (high << 8) + low;
                    }
                    else{ PC++; }
                    break;}
                case 0xD2:{
                    if (!get_C_flag()){
                        uint8_t low = read_byte(PC+1);
                        uint8_t high = read_byte(PC+2);
                        PC = (high << 8) + low;
                    }
                    else{ PC++; }
                    break;}
                case 0xE2:{
                    write_byte(get_C() + 0xFF00, get_A());
                    PC++;
                    break;}
                case 0xF2:{
                    write_byte(get_A() + 0xFF00, read_byte(get_C() + 0xFF00));
                    PC++;
                    break;}
                case 0x03:{
                    BC++;
                    PC++;
                    break;}
                case 0x13:{
                    DE++;
                    PC++;
                    break;}
                case 0x23:{
                    HL++;
                    PC++;
                    break;}
                case 0x33:{
                    SP++;
                    PC++;
                    break;}
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
                case 0xC3:{
                    uint8_t low = read_byte(PC+1);
                    uint8_t high = read_byte(PC+2);
                    PC = (high << 8) + low;
                    break;}
                case 0xF3:{
                    IME = 0;
                    PC++;
                    break;}
                case 0x04:{
                    uint8_t val = get_B() + 1;
                    set_B(val);
                    set_Z_flag(val == 0);
                    set_N_flag(false);
                    set_H_flag((val & 0x0F) == 0);
                    break;}
                case 0x14:{
                    uint8_t val = get_D() + 1;
                    set_D(val);
                    set_Z_flag(val == 0);
                    set_N_flag(false);
                    set_H_flag((val & 0x0F) == 0);
                    break;}
                case 0x24:{
                    uint8_t val = get_H() + 1;
                    set_H(val);
                    set_Z_flag(val == 0);
                    set_N_flag(false);
                    set_H_flag((val & 0x0F) == 0);
                    break;}
                case 0x34:{
                    uint8_t val = read_byte(HL) + 1;
                    write_byte(HL, val);
                    set_Z_flag(val == 0);
                    set_N_flag(false);
                    set_H_flag((val & 0x0F) == 0);
                    break;}
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
                case 0xC4:{
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
                    break;}
                case 0xD4:{
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
                    break;}
                case 0x05:{
                    uint8_t val = get_B() - 1;
                    set_B(val);
                    
                    set_Z_flag(val == 0);                 
                    set_N_flag(true);                     
                    set_H_flag((val & 0x0F) == 0x0F);
                    break;}
                case 0x15:{
                    uint8_t val = get_D() - 1;
                    set_D(val);
                    
                    set_Z_flag(val == 0);                 
                    set_N_flag(true);                     
                    set_H_flag((val & 0x0F) == 0x0F);
                    break;}
                case 0x25:{
                    uint8_t val = get_H() - 1;
                    set_H(val);
                    
                    set_Z_flag(val == 0);                 
                    set_N_flag(true);                     
                    set_H_flag((val & 0x0F) == 0x0F);
                    break;}
                case 0x35:{
                    uint8_t val = read_byte(HL) - 1;
                    write_byte(HL, val);
                    
                    set_Z_flag(val == 0);                 
                    set_N_flag(true);                     
                    set_H_flag((val & 0x0F) == 0x0F);
                    break;}
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
                case 0x85: {
                    uint8_t A = get_A();
                    uint8_t B = get_L();
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
                case 0x95: {
                    uint8_t A = get_A();
                    uint8_t B = get_L();
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
                case 0xA5: {
                    uint8_t A = get_A();
                    uint8_t B = get_L();
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
                case 0xB5: {
                    uint8_t A = get_A();
                    uint8_t B = get_L();
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
                case 0xC5: {
                    uint8_t low = BC & 0x00FF;
                    uint8_t high = (BC & 0xFF00) >> 8;

                    SP--;
                    write_byte(SP, high);
                    SP--;
                    write_byte(SP, low);
                    PC++;
                    break;
                }
                case 0xD5: {
                    uint8_t low = DE & 0x00FF;
                    uint8_t high = (DE & 0xFF00) >> 8;

                    SP--;
                    write_byte(SP, high);
                    SP--;
                    write_byte(SP, low);
                    PC++;
                    break;
                }
                case 0xE5: {
                    uint8_t low = HL & 0x00FF;
                    uint8_t high = (HL & 0xFF00) >> 8;

                    SP--;
                    write_byte(SP, high);
                    SP--;
                    write_byte(SP, low);
                    PC++;
                    break;
                }
                case 0xF5: {
                    uint8_t low = AF & 0x00FF;
                    uint8_t high = (AF & 0xFF00) >> 8;

                    SP--;
                    write_byte(SP, high);
                    SP--;
                    write_byte(SP, low);
                    PC++;
                    break;
                }
                case 0x06:{
                    uint8_t val = read_byte(PC+1);
                    set_B(val);
                    PC += 2;
                    break;}
                case 0x16:{
                    uint8_t val = read_byte(PC+1);
                    set_D(val);
                    PC += 2;
                    break;}
                case 0x26:{
                    uint8_t val = read_byte(PC+1);
                    set_H(val);
                    PC += 2;
                    break;}
                case 0x36:{
                    uint8_t val = read_byte(PC+1);
                    write_byte(HL, val);
                    PC += 2;
                    break;}
                case 0x46:{
                    uint8_t val = read_byte(HL);
                    set_B(val);
                    PC++;
                    break;}
                case 0x56:{
                    uint8_t val = read_byte(HL);
                    set_D(val);
                    PC++;
                    break;}
                case 0x66:{
                    uint8_t val = read_byte(HL);
                    set_H(val);
                    PC++;
                    break;}
                case 0x76:{
                    PC++;
                    break;}
                case 0x86: {
                    uint8_t A = get_A();
                    uint8_t B = read_byte(HL);
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
                case 0x96: {
                    uint8_t A = get_A();
                    uint8_t B = read_byte(HL);
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
                case 0xA6: {
                    uint8_t A = get_A();
                    uint8_t B = read_byte(HL);
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
                case 0xB6: {
                    uint8_t A = get_A();
                    uint8_t B = read_byte(HL);
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
                case 0xC6: {
                    uint8_t A = get_A();
                    uint8_t B = read_byte(PC+1);
                    uint8_t result = A + B;
                    
                    
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    if ((A & 0x0F) + (B & 0x0F) > 0x0F) { set_H_flag(1); }
                    else {set_H_flag(0);}
                    set_C_flag((result < A)); 

                    set_A(result);
                    PC+=2;
                    break;
                }
                case 0xD6: {
                    uint8_t A = get_A();
                    uint8_t B = read_byte(PC+1);
                    uint8_t result = A - B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(1);
                    set_C_flag((B > A)); 
                    if ((A & 0x0F) < (B & 0x0F)) { set_H_flag(1); }
                    else {set_H_flag(0);}

                    set_A(result);
                    PC+=2;
                    break;
                }
                case 0xE6: {
                    uint8_t A = get_A();
                    uint8_t B = read_byte(PC+1);
                    uint8_t result = A & B;

                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(1);
                    set_C_flag(0);

                    set_A(result);
                    PC+=2;
                    break;
                }
                case 0xF6: {
                    uint8_t A = get_A();
                    uint8_t B = read_byte(PC+1);
                    uint8_t result = A | B;
                    if (result != 0) {set_Z_flag(0);}
                    else {set_Z_flag(1);}
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0); 

                    set_A(result);
                    PC+=2;
                    break;
                }
                case 0x07: {
                    bool carry = (get_A() & 0x80) >> 7;
                    set_A(carry | (get_A() << 1));
                    set_C_flag(carry); 
                    set_Z_flag(false); 
                    set_N_flag(false);
                    set_H_flag(false);
                    PC++;
                    break;
                }
                case 0x17:{
                    bool carry = (get_A() & 0x80) >> 7;
                    set_A(get_C_flag() | (get_A() << 1));
                    set_C_flag(carry);
                    set_Z_flag(false); 
                    set_N_flag(false);
                    set_H_flag(false);
                    PC++;
                    break;
                }
                case 0x27:{
                    uint8_t adj = 0;
                    if (get_N_flag()){
                        if (get_H_flag()){
                            adj += 0x6;
                        }
                        if (get_C_flag()){
                            adj += 0x60;
                        }
                        uint8_t result = get_A() - adj;
                        if (result == 0) { set_Z_flag(true); }
                        else { set_Z_flag(false); }
                        set_A(result);
                    }
                    else {
                        if (get_H_flag() || ((get_A() & 0xF) > 0x9)){
                            adj += 0x6;
                        }
                        if (get_C_flag() || (get_A() > 0x99)){
                            adj += 0x60;
                            set_C_flag(true);
                        }
                        else{ set_C_flag(false); }
                        uint8_t result = get_A() + adj;
                        if (result == 0) { set_Z_flag(true); }
                        else { set_Z_flag(false); }
                        set_A(result);
                    }
                    set_H_flag(false);
                    PC++;
                    break;}
                case 0x37:{
                    set_C_flag(true);
                    set_H_flag(false);
                    set_N_flag(false);
                    PC++;
                    break;}
                case 0x47: {
                    set_B(get_A());
                    PC++;
                    break;
                }
                case 0x57: {
                    set_D(get_A());
                    PC++;
                    break;
                }
                case 0x67: {
                    set_H(get_A());
                    PC++;
                    break;
                }
                case 0x77: {
                    write_byte(HL, get_A());
                    PC++;
                    break;
                }
                case 0x87: {
                    uint8_t A = get_A();
                    uint8_t B = get_A();
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
                case 0x97: {
                    uint8_t A = get_A();
                    uint8_t B = get_A();
                    uint8_t result = A - B;

                    set_Z_flag(1);
                    set_N_flag(1);
                    set_C_flag(0);
                    set_H_flag(0);

                    set_A(result);
                    PC++;
                    break;
                }
                case 0xA7: {
                    uint8_t A = get_A();
                    uint8_t B = get_A();
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
                case 0xB7: {
                    uint8_t A = get_A();
                    uint8_t B = get_A();
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
                case 0xC7: {
                    uint16_t return_addr = PC + 1;
                    SP--;
                    write_byte(SP, (return_addr >> 8) & 0xFF);
                    SP--;
                    write_byte(SP, return_addr & 0xFF);
                    PC = 0x0000;
                    break;
                }
                case 0xD7: {
                    uint16_t return_addr = PC + 1;
                    SP--;
                    write_byte(SP, (return_addr >> 8) & 0xFF);
                    SP--;
                    write_byte(SP, return_addr & 0xFF);
                    PC = 0x0010;
                    break;
                }
                case 0xE7: {
                    uint16_t return_addr = PC + 1;
                    SP--;
                    write_byte(SP, (return_addr >> 8) & 0xFF);
                    SP--;
                    write_byte(SP, return_addr & 0xFF);
                    PC = 0x0020;
                    break;
                }
                case 0xF7: {
                    uint16_t return_addr = PC + 1;
                    SP--;
                    write_byte(SP, (return_addr >> 8) & 0xFF);
                    SP--;
                    write_byte(SP, return_addr & 0xFF);
                    PC = 0x0030;
                    break;
                }
                case 0x08:{
                    uint8_t low = read_byte(PC+1);
                    uint8_t high = read_byte(PC+2);
                    uint16_t address = (high << 8) + low;
                    write_byte(address, SP & 0xFF);
                    write_byte(address + 1, (SP >> 8) & 0xFF);
                    PC += 3;
                    break;
                }
                case 0x18:{
                    int8_t offset = read_byte(PC+1);
                    PC+=2;
                    PC += offset;
                    break;
                }
                case 0x28:{
                    int8_t offset = read_byte(PC+1);
                    PC+=2;
                    if (get_Z_flag()){
                        PC += offset;
                    }
                    break;
                }
                case 0x38: {
                    int8_t offset = read_byte(PC+1);
                    PC+=2;
                    if (get_C_flag()){
                        PC += offset;
                    }
                    break;
                }
                case 0x48: {
                    set_C(get_B());
                    PC++;
                    break;
                }
                case 0x58: {
                    set_E(get_B());
                    PC++;
                    break;
                }
                case 0x68: {
                    set_L(get_B());
                    PC++;
                    break;
                }
                case 0x78: {
                    set_A(get_B());
                    PC++;
                    break;
                }
                case 0x88: {
                    uint8_t a = get_A();
                    uint8_t b = get_B();
                    uint8_t c = get_C_flag();

                    uint16_t sum = (uint16_t)a + (uint16_t)b + (uint16_t)c;
                    uint8_t res = (uint8_t)sum;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(false);

                    set_H_flag(((a & 0x0F) + (b & 0x0F) + c) > 0x0F);

                    set_C_flag(sum > 0xFF);

                    PC++;
                    break;
                }
                case 0x98: {
                    uint8_t a = get_A();
                    uint8_t b = get_B();
                    uint8_t c = get_C_flag();

                    int16_t sub = (uint16_t)a - (uint16_t)b - (uint16_t)c;
                    uint8_t res = (uint8_t)sub;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag(((int) (a & 0x0F) - (int) (b & 0x0F) - (int) c) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xA8: {
                    uint8_t a = get_A();
                    uint8_t b = get_B();
                    uint8_t res = a ^ b;
                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0);
                    PC++;
                    break;
                }
                case 0xB8:{
                    uint8_t a = get_A();
                    uint8_t b = get_B();

                    int16_t sub = (uint16_t)a - (uint16_t)b;
                    uint8_t res = (uint8_t)sub;

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag((int) (a & 0x0F) - (int) (b & 0x0F) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xC8:{
                    if (get_Z_flag()){
                        uint8_t low = read_byte(SP);
                        SP++;
                        uint8_t high = read_byte(SP);
                        SP++;
                        PC = (high << 8) | low;
                    }
                    else{ PC++; }
                    break;
                }
                case 0xD8:{
                    if (get_C_flag()){
                        uint8_t low = read_byte(SP);
                        SP++;
                        uint8_t high = read_byte(SP);
                        SP++;
                        PC = (high << 8) | low;
                    }
                    else{ PC++; }
                    break;
                }
                case 0xE8:{
                    int8_t signed_a = (int8_t)read_byte(PC + 1);
                    uint8_t a = (uint8_t)signed_a;

                    int16_t sum = (int16_t)SP + (int16_t)signed_a;

                    set_Z_flag(false);
                    set_N_flag(false);

                    set_H_flag(((SP & 0x0F) + (a & 0x0F)) > 0x0F);

                    set_C_flag(((SP & 0xFF) + a) > 0xFF);

                    SP = (uint16_t)sum;
                    PC += 2;
                    break;
                }
                case 0xF8:{
                    int8_t signed_a = read_byte(PC+1);
                    uint8_t a = (uint8_t) signed_a;
                    int16_t sum = (int16_t)SP + (int16_t)signed_a;
                    HL = sum;

                    set_Z_flag(false);
                    set_N_flag(false);

                    set_H_flag(((SP & 0x0F) + (a & 0x0F)) > 0x0F);

                    set_C_flag(((SP & 0xFF) + a) > 0xFF);
                    PC += 2;
                    break;
                }
                case 0x09:{
                    uint32_t sum = (uint32_t)HL + (uint32_t)BC;

                    set_N_flag(false);

                    set_H_flag(((HL & 0xFFF) + (BC & 0xFFF)) > 0x0FFF);

                    set_C_flag(sum > 0xFFFF);

                    HL = (uint16_t)sum;
                    PC++;
                    break;
                }
                case 0x19:{
                    uint32_t sum = (uint32_t)HL + (uint32_t)DE;

                    set_N_flag(false);

                    set_H_flag(((HL & 0xFFF) + (DE & 0xFFF)) > 0x0FFF);

                    set_C_flag(sum > 0xFFFF);

                    HL = (uint16_t)sum;
                    PC++;
                    break;
                }
                case 0x29:{
                    uint32_t sum = (uint32_t)HL + (uint32_t)HL;

                    set_N_flag(false);

                    set_H_flag(((HL & 0xFFF) + (HL & 0xFFF)) > 0x0FFF);

                    set_C_flag(sum > 0xFFFF);

                    HL = (uint16_t)sum;
                    PC++;
                    break;
                }
                case 0x39:{
                    uint32_t sum = (uint32_t)HL + (uint32_t)SP;

                    set_N_flag(false);

                    set_H_flag(((HL & 0xFFF) + (SP & 0xFFF)) > 0x0FFF);

                    set_C_flag(sum > 0xFFFF);

                    HL = (uint16_t)sum;
                    PC++;
                    break;
                }
                case 0x49: {
                    set_C(get_C());
                    PC++;
                    break;
                }
                case 0x59: {
                    set_E(get_C());
                    PC++;
                    break;
                }
                case 0x69: {
                    set_L(get_C());
                    PC++;
                    break;
                }
                case 0x79: {
                    set_A(get_C());
                    PC++;
                    break;
                }
                case 0x89: {
                    uint8_t a = get_A();
                    uint8_t b = get_C();
                    uint8_t c = get_C_flag();

                    uint16_t sum = (uint16_t)a + (uint16_t)b + (uint16_t)c;
                    uint8_t res = (uint8_t)sum;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(false);

                    set_H_flag(((a & 0x0F) + (b & 0x0F) + c) > 0x0F);

                    set_C_flag(sum > 0xFF);

                    PC++;
                    break;
                }
                case 0x99: {
                    uint8_t a = get_A();
                    uint8_t b = get_C();
                    uint8_t c = get_C_flag();

                    int16_t sub = (uint16_t)a - (uint16_t)b - (uint16_t)c;
                    uint8_t res = (uint8_t)sub;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag(((int) (a & 0x0F) - (int) (b & 0x0F) - (int) c) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xA9: {
                    uint8_t a = get_A();
                    uint8_t b = get_C();
                    uint8_t res = a ^ b;
                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0);
                    PC++;
                    break;
                }
                case 0xB9:{
                    uint8_t a = get_A();
                    uint8_t b = get_C();

                    int16_t sub = (uint16_t)a - (uint16_t)b;
                    uint8_t res = (uint8_t)sub;

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag((int) (a & 0x0F) - (int) (b & 0x0F) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xC9:{
                    uint8_t low = read_byte(SP);
                    SP++;
                    uint8_t high = read_byte(SP);
                    SP++;
                    PC = (high << 8) | low;
                    break;
                }
                case 0xD9:{
                    IME = true;
                    uint8_t low = read_byte(SP);
                    SP++;
                    uint8_t high = read_byte(SP);
                    SP++;
                    PC = (high << 8) | low;
                    break;
                }
                case 0xE9:{
                    PC = HL;
                    break;
                }
                case 0xF9:{
                    SP = HL;
                    PC++;
                    break;
                }
                case 0x0A:{
                    set_A(read_byte(BC));
                    PC++;
                    break;
                }
                case 0x1A:{
                    set_A(read_byte(DE));
                    PC++;
                    break;
                }
                case 0x2A:{
                    set_A(read_byte(HL));
                    HL++;
                    PC++;
                    break;
                }
                case 0x3A:{
                    set_A(read_byte(HL));
                    HL--;
                    PC++;
                    break;
                }
                case 0x4A: {
                    set_C(get_D());
                    PC++;
                    break;
                }
                case 0x5A: {
                    set_E(get_D());
                    PC++;
                    break;
                }
                case 0x6A: {
                    set_L(get_D());
                    PC++;
                    break;
                }
                case 0x7A: {
                    set_A(get_D());
                    PC++;
                    break;
                }
                case 0x8A: {
                    uint8_t a = get_A();
                    uint8_t b = get_D();
                    uint8_t c = get_C_flag();

                    uint16_t sum = (uint16_t)a + (uint16_t)b + (uint16_t)c;
                    uint8_t res = (uint8_t)sum;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(false);

                    set_H_flag(((a & 0x0F) + (b & 0x0F) + c) > 0x0F);

                    set_C_flag(sum > 0xFF);

                    PC++;
                    break;
                }
                case 0x9A: {
                    uint8_t a = get_A();
                    uint8_t b = get_D();
                    uint8_t c = get_C_flag();

                    int16_t sub = (uint16_t)a - (uint16_t)b - (uint16_t)c;
                    uint8_t res = (uint8_t)sub;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag(((int) (a & 0x0F) - (int) (b & 0x0F) - (int) c) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xAA: {
                    uint8_t a = get_A();
                    uint8_t b = get_D();
                    uint8_t res = a ^ b;
                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0);
                    PC++;
                    break;
                }
                case 0xBA:{
                    uint8_t a = get_A();
                    uint8_t b = get_D();

                    int16_t sub = (uint16_t)a - (uint16_t)b;
                    uint8_t res = (uint8_t)sub;

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag((int) (a & 0x0F) - (int) (b & 0x0F) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xCA: {
                    if (get_Z_flag()){
                        uint8_t low = read_byte(PC+1);
                        uint8_t high = read_byte(PC+2);
                        uint16_t address = (high << 8) | low;
                        PC = address;
                    }
                    else { PC+=3; }
                    break;
                }
                case 0xDA: {
                    if (get_C_flag()){
                        uint8_t low = read_byte(PC+1);
                        uint8_t high = read_byte(PC+2);
                        uint16_t address = (high << 8) | low;
                        PC = address;
                    }
                    else { PC+=3; }
                    break;
                }
                case 0xEA: {
                    uint8_t low = read_byte(PC+1);
                    uint8_t high = read_byte(PC+2);
                    uint16_t address = (high << 8) | low;
                    write_byte(address, get_A());
                    PC+=3;
                    break;
                }
                case 0xFA: {
                    uint8_t low = read_byte(PC+1);
                    uint8_t high = read_byte(PC+2);
                    uint16_t address = (high << 8) | low;
                    set_A(read_byte(address));
                    PC+=3;
                    break;
                }
                case 0x0B:{
                    BC--;
                    PC++;
                    break;
                }
                case 0x1B:{
                    DE--;
                    PC++;
                    break;
                }
                case 0x2B:{
                    HL--;
                    PC++;
                    break;
                }
                case 0x3B:{
                    SP--;
                    PC++;
                    break;
                }
                case 0x4B: {
                    set_C(get_E());
                    PC++;
                    break;
                }
                case 0x5B: {
                    set_E(get_E());
                    PC++;
                    break;
                }
                case 0x6B: {
                    set_L(get_E());
                    PC++;
                    break;
                }
                case 0x7B: {
                    set_A(get_E());
                    PC++;
                    break;
                }
                case 0x8B: {
                    uint8_t a = get_A();
                    uint8_t b = get_E();
                    uint8_t c = get_C_flag();

                    uint16_t sum = (uint16_t)a + (uint16_t)b + (uint16_t)c;
                    uint8_t res = (uint8_t)sum;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(false);

                    set_H_flag(((a & 0x0F) + (b & 0x0F) + c) > 0x0F);

                    set_C_flag(sum > 0xFF);

                    PC++;
                    break;
                }
                case 0x9B: {
                    uint8_t a = get_A();
                    uint8_t b = get_E();
                    uint8_t c = get_C_flag();

                    int16_t sub = (uint16_t)a - (uint16_t)b - (uint16_t)c;
                    uint8_t res = (uint8_t)sub;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag(((int) (a & 0x0F) - (int) (b & 0x0F) - (int) c) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xAB: {
                    uint8_t a = get_A();
                    uint8_t b = get_E();
                    uint8_t res = a ^ b;
                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0);
                    PC++;
                    break;
                }
                case 0xBB:{
                    uint8_t a = get_A();
                    uint8_t b = get_E();

                    int16_t sub = (uint16_t)a - (uint16_t)b;
                    uint8_t res = (uint8_t)sub;

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag((int) (a & 0x0F) - (int) (b & 0x0F) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xCB:{
                    std::cout << "prefix requested" << std::endl;
                    return 1;
                }
                case 0xFB: {
                    IME = true;
                    PC++;
                    break;
                }
                case 0x0C: {
                    uint8_t c = get_C();
                    uint8_t res = c + 1;

                    set_C(res);

                    set_Z_flag(res == 0);
                    set_N_flag(false);
                    set_H_flag((c & 0x0F) == 0x0F);
                    
                    PC++;
                    break;
                }
                case 0x1C: {
                    uint8_t c = get_E();
                    uint8_t res = c + 1;

                    set_E(res);
                    
                    set_Z_flag(res == 0);
                    set_N_flag(false);
                    set_H_flag((c & 0x0F) == 0x0F);
                    
                    PC++;
                    break;
                }
                case 0x2C: {
                    uint8_t c = get_L();
                    uint8_t res = c + 1;

                    set_L(res);
                    
                    set_Z_flag(res == 0);
                    set_N_flag(false);
                    set_H_flag((c & 0x0F) == 0x0F);
                    
                    PC++;
                    break;
                }
                case 0x3C: {
                    uint8_t c = get_A();
                    uint8_t res = c + 1;

                    set_A(res);
                    
                    set_Z_flag(res == 0);
                    set_N_flag(false);
                    set_H_flag((c & 0x0F) == 0x0F);
                    
                    PC++;
                    break;
                }
                case 0x4C: {
                    set_C(get_H());
                    PC++;
                    break;
                }
                case 0x5C: {
                    set_E(get_H());
                    PC++;
                    break;
                }
                case 0x6C: {
                    set_L(get_H());
                    PC++;
                    break;
                }
                case 0x7C: {
                    set_A(get_H());
                    PC++;
                    break;
                }
                case 0x8C: {
                    uint8_t a = get_A();
                    uint8_t b = get_H();
                    uint8_t c = get_C_flag();

                    uint16_t sum = (uint16_t)a + (uint16_t)b + (uint16_t)c;
                    uint8_t res = (uint8_t)sum;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(false);

                    set_H_flag(((a & 0x0F) + (b & 0x0F) + c) > 0x0F);

                    set_C_flag(sum > 0xFF);

                    PC++;
                    break;
                }
                case 0x9C: {
                    uint8_t a = get_A();
                    uint8_t b = get_H();
                    uint8_t c = get_C_flag();

                    int16_t sub = (uint16_t)a - (uint16_t)b - (uint16_t)c;
                    uint8_t res = (uint8_t)sub;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag(((int) (a & 0x0F) - (int) (b & 0x0F) - (int) c) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xAC: {
                    uint8_t a = get_A();
                    uint8_t b = get_H();
                    uint8_t res = a ^ b;
                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0);
                    PC++;
                    break;
                }
                case 0xBC:{
                    uint8_t a = get_A();
                    uint8_t b = get_H();

                    int16_t sub = (uint16_t)a - (uint16_t)b;
                    uint8_t res = (uint8_t)sub;

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag((int) (a & 0x0F) - (int) (b & 0x0F) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xCC:{
                    if (get_Z_flag()){
                        uint8_t low = read_byte(PC+1);
                        uint8_t high = read_byte(PC+2);
                        uint16_t address = (high << 8) | low;
                        uint16_t post_call_address = PC+3;
                        
                        SP--;
                        write_byte(SP, post_call_address >> 8);
                        SP--;
                        write_byte(SP, post_call_address & 0x00FF);
                        PC = address;
                    }
                    else{ PC+=3; }
                    break;
                }
                case 0xDC:{
                    if (get_C_flag()){
                        uint8_t low = read_byte(PC+1);
                        uint8_t high = read_byte(PC+2);
                        uint16_t address = (high << 8) | low;
                        uint16_t post_call_address = PC+3;
                        
                        SP--;
                        write_byte(SP, post_call_address >> 8);
                        SP--;
                        write_byte(SP, post_call_address & 0x00FF);
                        PC = address;
                    }
                    else{ PC+=3; }
                    break;
                }
                case 0x0D:{
                    uint8_t c = get_C();
                    uint8_t res = c - 1;

                    set_Z_flag(res == 0);
                    set_N_flag(1);
                    set_H_flag((c & 0xF) == 0);

                    PC++;
                    break;
                }
                case 0x1D:{
                    uint8_t c = get_E();
                    uint8_t res = c - 1;

                    set_Z_flag(res == 0);
                    set_N_flag(1);
                    set_H_flag((c & 0xF) == 0);

                    PC++;
                    break;
                }
                case 0x2D:{
                    uint8_t c = get_L();
                    uint8_t res = c - 1;

                    set_Z_flag(res == 0);
                    set_N_flag(1);
                    set_H_flag((c & 0xF) == 0);

                    PC++;
                    break;
                }
                case 0x3D:{
                    uint8_t c = get_A();
                    uint8_t res = c - 1;

                    set_Z_flag(res == 0);
                    set_N_flag(1);
                    set_H_flag((c & 0xF) == 0);

                    PC++;
                    break;
                }
                case 0x4D: {
                    set_C(get_L());
                    PC++;
                    break;
                }
                case 0x5D: {
                    set_E(get_L());
                    PC++;
                    break;
                }
                case 0x6D: {
                    set_L(get_L());
                    PC++;
                    break;
                }
                case 0x7D: {
                    set_A(get_L());
                    PC++;
                    break;
                }
                case 0x8D: {
                    uint8_t a = get_A();
                    uint8_t b = get_L();
                    uint8_t c = get_C_flag();

                    uint16_t sum = (uint16_t)a + (uint16_t)b + (uint16_t)c;
                    uint8_t res = (uint8_t)sum;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(false);

                    set_H_flag(((a & 0x0F) + (b & 0x0F) + c) > 0x0F);

                    set_C_flag(sum > 0xFF);

                    PC++;
                    break;
                }
                case 0x9D: {
                    uint8_t a = get_A();
                    uint8_t b = get_L();
                    uint8_t c = get_C_flag();

                    int16_t sub = (uint16_t)a - (uint16_t)b - (uint16_t)c;
                    uint8_t res = (uint8_t)sub;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag(((int) (a & 0x0F) - (int) (b & 0x0F) - (int) c) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xAD: {
                    uint8_t a = get_A();
                    uint8_t b = get_L();
                    uint8_t res = a ^ b;
                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0);
                    PC++;
                    break;
                }
                case 0xBD:{
                    uint8_t a = get_A();
                    uint8_t b = get_L();

                    int16_t sub = (uint16_t)a - (uint16_t)b;
                    uint8_t res = (uint8_t)sub;

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag((int) (a & 0x0F) - (int) (b & 0x0F) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xCD:{

                    uint8_t low = read_byte(PC+1);
                    uint8_t high = read_byte(PC+2);
                    uint16_t address = (high << 8) | low;
                    uint16_t post_call_address = PC+3;
                    
                    SP--;
                    write_byte(SP, post_call_address >> 8);
                    SP--;
                    write_byte(SP, post_call_address & 0x00FF);
                    PC = address;
                    
                    break;
                }
                case 0x0E: {
                    uint8_t val = read_byte(PC+1);
                    set_C(val);
                    PC+=2;
                    break;
                }
                case 0x1E: {
                    uint8_t val = read_byte(PC+1);
                    set_E(val);
                    PC+=2;
                    break;
                }
                case 0x2E: {
                    uint8_t val = read_byte(PC+1);
                    set_L(val);
                    PC+=2;
                    break;
                }
                case 0x3E: {
                    uint8_t val = read_byte(PC+1);
                    set_A(val);
                    PC+=2;
                    break;
                }
                case 0x4E:{
                    set_C(read_byte(HL));
                    PC++;
                    break;
                }
                case 0x5E:{
                    set_E(read_byte(HL));
                    PC++;
                    break;
                }
                case 0x6E:{
                    set_L(read_byte(HL));
                    PC++;
                    break;
                }
                case 0x7E:{
                    set_A(read_byte(HL));
                    PC++;
                    break;
                }
                case 0x8E: {
                    uint8_t a = get_A();
                    uint8_t b = read_byte(HL);
                    uint8_t c = get_C_flag();

                    uint16_t sum = (uint16_t)a + (uint16_t)b + (uint16_t)c;
                    uint8_t res = (uint8_t)sum;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(false);

                    set_H_flag(((a & 0x0F) + (b & 0x0F) + c) > 0x0F);

                    set_C_flag(sum > 0xFF);

                    PC++;
                    break;
                }
                case 0x9E: {
                    uint8_t a = get_A();
                    uint8_t b = read_byte(HL);
                    uint8_t c = get_C_flag();

                    int16_t sub = (uint16_t)a - (uint16_t)b - (uint16_t)c;
                    uint8_t res = (uint8_t)sub;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag(((int) (a & 0x0F) - (int) (b & 0x0F) - (int) c) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xAE: {
                    uint8_t a = get_A();
                    uint8_t b = read_byte(HL);
                    uint8_t res = a ^ b;
                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0);
                    PC++;
                    break;
                }
                case 0xBE:{
                    uint8_t a = get_A();
                    uint8_t b = read_byte(HL);

                    int16_t sub = (uint16_t)a - (uint16_t)b;
                    uint8_t res = (uint8_t)sub;

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag((int) (a & 0x0F) - (int) (b & 0x0F) < 0);

                    set_C_flag(sub < 0);

                    PC++;
                    break;
                }
                case 0xCE: {
                    uint8_t a = get_A();
                    uint8_t b = read_byte(PC+1);
                    uint8_t c = get_C_flag();

                    uint16_t sum = (uint16_t)a + (uint16_t)b + (uint16_t)c;
                    uint8_t res = (uint8_t)sum;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(false);

                    set_H_flag(((a & 0x0F) + (b & 0x0F) + c) > 0x0F);

                    set_C_flag(sum > 0xFF);

                    PC+=2;
                    break;
                }
                case 0xDE: {
                    uint8_t a = get_A();
                    uint8_t b = read_byte(PC+1);
                    uint8_t c = get_C_flag();

                    int16_t sub = (uint16_t)a - (uint16_t)b - (uint16_t)c;
                    uint8_t res = (uint8_t)sub;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag(((int) (a & 0x0F) - (int) (b & 0x0F) - (int) c) < 0);

                    set_C_flag(sub < 0);

                    PC+=2;
                    break;
                }
                case 0xEE: {
                    uint8_t a = get_A();
                    uint8_t b = read_byte(PC+1);
                    uint8_t res = a ^ b;
                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0);
                    PC+=2;
                    break;
                }
                case 0xFE:{
                    uint8_t a = get_A();
                    uint8_t b = read_byte(PC+1);

                    int16_t sub = (uint16_t)a - (uint16_t)b;
                    uint8_t res = (uint8_t)sub;

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag((int) (a & 0x0F) - (int) (b & 0x0F) < 0);

                    set_C_flag(sub < 0);

                    PC+=2;
                    break;
                }
                case 0x0F:{
                    bool carry = get_A() & 0x1;
                    uint8_t res = (get_A() >> 1) | (carry ? 0x80 : 0);

                    set_A(res);
                    set_Z_flag(0);
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(carry);

                    PC++;
                    break;
                }
                case 0x1F: {
                    uint8_t a = get_A();
                    bool old_carry = get_C_flag();
                    bool new_carry = a & 0x01;

                    uint8_t res = (a >> 1) | (old_carry ? 0x80 : 0);

                    set_A(res);
                    
                    set_Z_flag(false);
                    set_N_flag(false);
                    set_H_flag(false);
                    set_C_flag(new_carry);

                    PC++;
                    break;
                }
                case 0x2F:{
                    set_A(~get_A());
                    set_N_flag(true);
                    set_H_flag(true);
                    PC++;
                    break;
                }
                case 0x3F:{
                    set_N_flag(false);
                    set_H_flag(false);
                    set_C_flag(!get_C_flag());

                    PC++;
                    break;
                }
                case 0x4F:{
                    set_C(get_A());
                    PC++;
                    break;
                }
                case 0x5F:{
                    set_E(get_A());
                    PC++;
                    break;
                }
                case 0x6F:{
                    set_L(get_A());
                    PC++;
                    break;
                }
                case 0x7F:{
                    set_A(get_A());
                    PC++;
                    break;
                }
                case 0x8F: {
                    uint8_t a = get_A();
                    uint8_t b = get_A();
                    uint8_t c = get_C_flag();

                    uint16_t sum = (uint16_t)a + (uint16_t)b + (uint16_t)c;
                    uint8_t res = (uint8_t)sum;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(false);

                    set_H_flag(((a & 0x0F) + (b & 0x0F) + c) > 0x0F);

                    set_C_flag(sum > 0xFF);

                    PC++;
                    break;
                }
                case 0x9F: {
                    uint8_t a = get_A();
                    uint8_t b = get_A();
                    uint8_t c = get_C_flag();

                    int16_t sub = (uint16_t)a - (uint16_t)b - (uint16_t)c;
                    uint8_t res = (uint8_t)sub;

                    set_A(res);

                    set_Z_flag(res == 0);
                    set_N_flag(true);

                    set_H_flag(((int) (a & 0x0F) - (int) (b & 0x0F) - (int) c) < 0);


                    PC++;
                    break;
                }
                case 0xAF: {
                    uint8_t a = get_A();
                    uint8_t b = get_A();
                    uint8_t res = a ^ b;
                    set_A(res);

                    set_Z_flag(1);
                    set_N_flag(0);
                    set_H_flag(0);
                    set_C_flag(0);
                    PC++;
                    break;
                }
                case 0xBF:{
                    uint8_t a = get_A();
                    uint8_t b = get_A();

                    int16_t sub = (uint16_t)a - (uint16_t)b;
                    uint8_t res = (uint8_t)sub;

                    set_Z_flag(1);
                    set_N_flag(true);

                    set_H_flag(0);

                    set_C_flag(0);

                    PC++;
                    break;
                }
                case 0xCF: {
                    uint16_t return_addr = PC + 1;
                    SP--;
                    write_byte(SP, (return_addr >> 8) & 0xFF);
                    SP--;
                    write_byte(SP, return_addr & 0xFF);
                    PC = 0x0008;
                    break;
                }
                case 0xDF: {
                    uint16_t return_addr = PC + 1;
                    SP--;
                    write_byte(SP, (return_addr >> 8) & 0xFF);
                    SP--;
                    write_byte(SP, return_addr & 0xFF);
                    PC = 0x0018;
                    break;
                }
                case 0xEF: {
                    uint16_t return_addr = PC + 1;
                    SP--;
                    write_byte(SP, (return_addr >> 8) & 0xFF);
                    SP--;
                    write_byte(SP, return_addr & 0xFF);
                    PC = 0x0028;
                    break;
                }
                case 0xFF: {
                    uint16_t return_addr = PC + 1;
                    SP--;
                    write_byte(SP, (return_addr >> 8) & 0xFF);
                    SP--;
                    write_byte(SP, return_addr & 0xFF);
                    PC = 0x0038;
                    break;
                }
            }
        }

        // Close the file stream explicitly (though it closes automatically when out of scope)
        rom_file.close();

        return 0;
    }
};

int main(){
    GameBoy gb = GameBoy();
    gb.run();
}