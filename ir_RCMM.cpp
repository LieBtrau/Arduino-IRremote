#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                      RRRRRR   CCCCC   MM    MM MM    MM
//                      RR   RR CC    C  MMM  MMM MMM  MMM
//                      RRRRRR  CC       MM MM MM MM MM MM
//                      RR  RR  CC    C  MM    MM MM    MM
//                      RR   RR  CCCCC   MM    MM MM    MM
//==============================================================================

#define RCMM_BITS 32
#define RCMM_HDR_MARK 425
#define RCMM_HDR_SPACE 260
#define RCMM_BIT_MARK 150
#define RCMM_00_SPACE 294
#define RCMM_01_SPACE 450
#define RCMM_10_SPACE 610
#define RCMM_11_SPACE 791
#define RCMM_RPT_LENGTH 100000

#define RCMM_HDR_MARK_PLS 16
#define RCMM_HDR_SPACE_PLS 10
#define RCMM_BIT_MARK_PLS 6
#define RCMM_00_SPACE_PLS 10
#define RCMM_01_SPACE_PLS 16
#define RCMM_10_SPACE_PLS 22
#define RCMM_11_SPACE_PLS 28

//+=============================================================================
// Be sure to use an IR-receiver that is fast enough for the short bursts, e.g. TSOP94336.
// TSOP1736 will NOT work.
//

#if SEND_RCMM
void IRsend::sendRCMM(unsigned long data, int nbits)
{
	// Set IR carrier frequency
	enableIROut(36);

	markPulses(RCMM_HDR_MARK_PLS);
	spaceDeadPulses(RCMM_HDR_SPACE_PLS);

	// Data
	for (byte i = 16; i > 0; i--)
	{
		markPulses(RCMM_BIT_MARK_PLS);
		switch (data & 0xC0000000UL)
		{
		case 0:
			spaceDeadPulses(RCMM_00_SPACE_PLS);
			break;
		case 0x40000000UL:
			spaceDeadPulses(RCMM_01_SPACE_PLS);
			break;
		case 0x80000000UL:
			spaceDeadPulses(RCMM_10_SPACE_PLS);
			break;
		case 0xC0000000UL:
			spaceDeadPulses(RCMM_11_SPACE_PLS);
			break;
		default:
			break;
		}
		data <<= 2;
	}

	// Footer
	markPulses(RCMM_BIT_MARK_PLS);

	/*
 * Alternative version : timing less under control
 */
	// // Set IR carrier frequency
	// enableIROut(36);

	// mark(RCMM_HDR_MARK);
	// space(RCMM_HDR_SPACE);

	// // Data
	// for (byte i = 16; i > 0; i--)
	// {
	// 	mark(RCMM_BIT_MARK);
	// 	switch (data & 0xC0000000UL)
	// 	{
	// 	case 0:
	// 		space(RCMM_00_SPACE);
	// 		break;
	// 	case 0x40000000UL:
	// 		space(RCMM_01_SPACE);
	// 		break;
	// 	case 0x80000000UL:
	// 		space(RCMM_10_SPACE);
	// 		break;
	// 	case 0xC0000000UL:
	// 		space(RCMM_11_SPACE);
	// 		break;
	// 	default:
	// 		break;
	// 	}
	// 	data <<= 2;
	// }

	// // Footer
	// mark(RCMM_BIT_MARK_PLS);
	// space(0);
}
#endif

//+=============================================================================
#if DECODE_RCMM
bool IRrecv::decodeRCMM(decode_results *results)
{
	long data = 0;
	int offset = 1; // Skip first space

	// Initial mark
	if (!MATCH_MARK(results->rawbuf[offset++], RCMM_HDR_MARK))
		return false;

	// Only decode once full packet is captured
	if (irparams.rawlen < RCMM_BITS + 1)
		return false;

	// Initial space
	if (!MATCH_SPACE(results->rawbuf[offset++], RCMM_HDR_SPACE))
		return false;

	for (int i = 0; i < RCMM_BITS / 2; i++)
	{
		if (!MATCH_MARK(results->rawbuf[offset++], RCMM_BIT_MARK))
			return false;

		if (MATCH_SPACE(results->rawbuf[offset], RCMM_00_SPACE))
			data <<= 2;
		else if (MATCH_SPACE(results->rawbuf[offset], RCMM_01_SPACE))
			data = (data << 2) | 1;
		else if (MATCH_SPACE(results->rawbuf[offset], RCMM_10_SPACE))
			data = (data << 2) | 2;
		else if (MATCH_SPACE(results->rawbuf[offset], RCMM_11_SPACE))
			data = (data << 2) | 3;
		else
			return false;
		offset++;
	}

	// Stop bit
	if (!MATCH_MARK(results->rawbuf[offset], RCMM_BIT_MARK))
		return false;

	// Success
	results->bits = RCMM_BITS;
	results->value = data;
	results->decode_type = RCMM;

	return true;
}
#endif