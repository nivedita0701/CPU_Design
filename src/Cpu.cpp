#include "Cpu.h"
#include <sstream>

Cpu::Cpu(Memory& memory_unit) : mem(memory_unit) {
    registers[SP] = STACK_START; 
    registers[FP] = STACK_START;
}

void Cpu::set_flag(uint8_t flag_bit, bool value) {
    if (value) flags |= flag_bit;
    else flags &= ~flag_bit;
}

void Cpu::fetch() {
    if (PC < MEM_SIZE - 1 && PC >= CODE_START) {
        IR = mem.read_word(PC);
        PC += WORD_SIZE; 
    } else {
        halted = true;
    }
}

void Cpu::decode_execute() {
    uint8_t opcode     = (IR >> 12) & 0xF;
    uint8_t r_dest_idx = (IR >> 8)  & 0xF;
    uint8_t r_src1_idx = (IR >> 4)  & 0xF;
    uint8_t r_src2_idx =  IR        & 0xF;
    uint8_t immediate_8bit = IR & 0xFF; 
    uint16_t address_12bit = IR & 0x0FFF;

    auto fmt16 = [](uint16_t v) {
        std::ostringstream o;
        o << "0x" << std::uppercase << std::hex
          << std::setw(4) << std::setfill('0') << v;
        return o.str();
    };

    auto trace = [&](const char* mnem){
        std::cout << "[PC=" << fmt16(PC - WORD_SIZE)
                  << "] " << mnem
                  << " | R0=" << fmt16(registers[R0])
                  << " R1=" << fmt16(registers[R1])
                  << " SP=" << fmt16(registers[SP])
                  << " FP=" << fmt16(registers[FP])
                  << " ZN=" << ((flags & FLAG_ZERO) ? '1' : '0')
                             << ((flags & FLAG_NEGATIVE) ? '1' : '0')
                  << "\n";
    };

    switch (opcode) {

        case OP_LOADI: { // LOADI RDest, #Immediate (8-bit)
            trace("LOADI");
            registers[r_dest_idx] = immediate_8bit;
            break;
        }
        case OP_LOAD: { // LOAD RDest, [RAddr] (RAddr is R_Src1)
            trace("LOAD");
            uint16_t address = registers[r_src1_idx];
            registers[r_dest_idx] = mem.read_word(address);
            break;
        }
        case OP_STORE: { // STORE RDest, [RAddr]
            trace("STORE");
            uint16_t store_data = registers[r_dest_idx];
            uint16_t address = registers[r_src1_idx];
            mem.write_word(address, store_data);
            break;
        }
        case OP_ADD: { // ADD RDest, RSrc1, RSrc2
            trace("ADD");
            uint16_t result = registers[r_src1_idx] + registers[r_src2_idx];
            registers[r_dest_idx] = result;
            set_flag(FLAG_ZERO, result == 0);
            set_flag(FLAG_NEGATIVE, (result & 0x8000) != 0);
            break;
        }
        case OP_ADDI: { // ADDI RDest, RSrc, #imm4  (imm4 is SIGNED: -8..+7)
            trace("ADDI");
            int16_t imm4 = (IR & 0x0008)
                         ? (int16_t)((IR & 0x000F) | 0xFFF0)
                         : (int16_t)(IR & 0x000F);
            int32_t tmp = (int32_t)registers[r_src1_idx] + (int32_t)imm4;
            registers[r_dest_idx] = (uint16_t)(tmp & 0xFFFF);
            set_flag(FLAG_ZERO, registers[r_dest_idx] == 0);
            set_flag(FLAG_NEGATIVE, (registers[r_dest_idx] & 0x8000) != 0);
            break;
        }
        case OP_JUMP: { // JUMP Address (12-bit)
            trace("JUMP");
            PC = address_12bit;
            break;
        }
        case OP_CMP: { // CMP RDest, RSrc
            trace("CMP");
            int16_t a = static_cast<int16_t>(registers[r_dest_idx]);
            int16_t b = static_cast<int16_t>(registers[r_src1_idx]);
            set_flag(FLAG_ZERO, a == b);
            set_flag(FLAG_NEGATIVE, a < b);
            break;
        }
        case OP_JUMPEQ: { // JUMPEQ Address (12-bit)
            trace("JUMPEQ");
            if (flags & FLAG_ZERO) {
                PC = address_12bit;
            }
            break;
        }
        case OP_HALT: {
            trace("HALT");
            halted = true;
            break;
        }
        default:
            std::cerr << "ERROR: Unknown opcode 0x"
                      << std::hex << (int)opcode
                      << " at PC 0x" << (PC - WORD_SIZE) << "\n";
            halted = true;
            break;
    }
}

void Cpu::run() {
    std::cout << "--- CPU Running ---\n";
    while (!halted) {
        fetch();
        if (!halted) decode_execute();
    }
    std::cout << "\n--- CPU Halted ---\n";
}

void Cpu::step() {
    if (!halted) {
        fetch();
        if (!halted) decode_execute();
    }
} 

void Cpu::print_registers() const {
    std::cout << std::hex << std::setfill('0');
    std::cout << "\n--- Registers ---\n";
    std::cout << "PC: 0x" << std::setw(4) << PC
              << " | IR: 0x" << std::setw(4) << IR
              << " | Flags: 0x" << std::setw(2) << (int)flags << "\n";
    for (int i = 0; i < NUM_REGISTERS; ++i) {
        std::string name = (i == SP) ? "SP" : (i == FP) ? "FP" : "R" + std::to_string(i);
        std::cout << name << ": 0x" << std::setw(4) << registers[i];
        if ((i + 1) % 4 == 0) std::cout << "\n";
        else std::cout << " | ";
    }
    std::cout << std::dec << "\n";
}
