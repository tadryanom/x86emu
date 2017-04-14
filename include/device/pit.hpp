#ifndef _PIT_H
#define _PIT_H

#include "common.hpp"
#include "dev_irq.hpp"
#include "dev_io.hpp"

class PIT : public IRQ, public PortIO {
	private:
		union {
			uint8_t raw;
			struct {
				uint8_t BCD : 1;
				uint8_t mode : 3;
				uint8_t RL : 2;
				uint8_t SC : 2;
			};
		} cwr;

		uint16_t count[3];
		uint16_t def[3];
		bool first;
	public:
		PIT();
		uint8_t in8(uint16_t addr);
		void out8(uint16_t addr, uint8_t v);
		//bool chk_intreq(void);
		void counter(void);
};

#endif