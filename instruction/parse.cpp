#include <string.h>
#include <map>
#include "instruction/instruction.hpp"

uint8_t ParseInstr::chk_chsz(void){
	uint8_t code, chsz;

	while(true){
		code = get_emu()->get_code8(0);
		switch(code){
			case 0x66:
				chsz |= CHSZ_OP;
				UPDATE_EIP(1);
				break;
			case 0x67:
				chsz |= CHSZ_AD;
				UPDATE_EIP(1);
				break;
			default:
				return chsz;
		}
	}
}

void ParseInstr::parse(void){
	memset(&instr, 0, sizeof(instr));

	parse_prefix_opcode();

	if(!chk.count(OPCODE)){
		DEBUG_MSG("\n");
		ERROR("no opecode : %x", OPCODE);
	}

	if(chk[OPCODE].modrm)
		parse_modrm_sib_disp();

	if(chk[OPCODE].imm32){
		IMM32 = get_emu()->get_code32(0);
		DEBUG_MSG("imm32:0x%02x ", IMM32);
		UPDATE_EIP(4);
	}
	else if(chk[OPCODE].imm16){
		IMM16 = get_emu()->get_code16(0);
		DEBUG_MSG("imm16:0x%02x ", IMM16);
		UPDATE_EIP(2);
	}
	else if(chk[OPCODE].imm8){
		IMM8 = (int8_t)get_emu()->get_code8(0);
		DEBUG_MSG("imm8:0x%02x ", IMM8);
		UPDATE_EIP(1);
	}

	if(chk[OPCODE].ptr16){
		PTR16 = get_emu()->get_code16(0);
		DEBUG_MSG("ptr16:0x%02x", PTR16);
		UPDATE_EIP(2);
	}

	DEBUG_MSG("\n");
}

void ParseInstr::parse_prefix_opcode(void){
	uint8_t code;

	code = get_emu()->get_code8(0);
	UPDATE_EIP(1);

	// prefix
	switch(code){
		case 0x26:
			PRE_SEGMENT = ES;
			goto next;
		case 0x2e:
			PRE_SEGMENT = CS;
			goto next;
		case 0x36:
			PRE_SEGMENT = SS;
			goto next;
		case 0x3e:
			PRE_SEGMENT = DS;
			goto next;
		case 0x64:
			PRE_SEGMENT = FS;
			goto next;
		case 0x65:
			PRE_SEGMENT = GS;
next:			PREFIX = code;
			code = get_emu()->get_code8(0);
			UPDATE_EIP(1);
	}

	OPCODE = code;

	// two byte opcode
	if(OPCODE == 0x0f){
		OPCODE = (OPCODE << 8) + get_emu()->get_code8(0);
		UPDATE_EIP(1);
	}

	if(is_protected())
		DEBUG_MSG("CS:%04x EIP:0x%04x opcode:%02x ", EMU->get_sgreg(CS), GET_EIP()-1, OPCODE);
	else
		DEBUG_MSG("CS:%04x  IP:0x%04x opcode:%02x ", EMU->get_sgreg(CS), GET_IP()-1, OPCODE);
}

void ParseInstr::parse_modrm_sib_disp(void){
	_MODRM = get_emu()->get_code8(0);
	UPDATE_EIP(1);

	DEBUG_MSG("[mod:0x%02x reg:0x%02x rm:0x%02x] ", MOD, REG, RM);

	if(is_protected() ^ chsz_ad){
		if (MOD != 3 && RM == 4) {
			_SIB = get_emu()->get_code8(0);
			UPDATE_EIP(1);
			DEBUG_MSG("[scale:0x%02x index:0x%02x base:0x%02x] ", SCALE, INDEX, BASE);
		}

		if (MOD == 2 || (MOD == 0 && RM == 5) || (MOD == 0 && BASE == 5)) {
			DISP32 = get_emu()->get_code32(0);
			UPDATE_EIP(4);
			DEBUG_MSG("disp32:0x%02x ", DISP32);
		}
		else if (MOD == 1) {
			DISP8 = (int8_t)get_emu()->get_code8(0);
			UPDATE_EIP(1);
			DEBUG_MSG("disp8:0x%02x ", DISP8);
		}
	}
	else{
		if ((MOD == 0 && RM == 6) || MOD == 2) {
			DISP16 = get_emu()->get_code32(0);
			UPDATE_EIP(2);
			DEBUG_MSG("disp16:0x%02x ", DISP16);
		}
		else if (MOD == 1) {
			DISP8 = (int8_t)get_emu()->get_code8(0);
			UPDATE_EIP(1);
			DEBUG_MSG("disp8:0x%02x ", DISP8);
		}
	}
}
