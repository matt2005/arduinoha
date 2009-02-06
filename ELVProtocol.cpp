#include "ELVProtocol.h"
#include <hardwareserial.h>

//ELV Funk-Innen-Aussensensor ASH 2200-1, HFS 868,35MHz. On-Off-Keying, Received with: Aurel RX-8L50SA70SF	
//[  24]   50 [  54]   23 [  54]   22 [  54]   22 [  58]   18 [  54]   28 [  48]   23 [  54]   23 [  56]   20 [  54]   22 [  23]   53 [  23]   54 [  53]   23 [  58]   19 [  58]   18 [  22]   54 [  23]   54 [  23]   52 [  54]   23 [  54]   23 [  22]   58 [  18]   54 [  23]   53 [  23]   54 [  57]   19 [  22]   55 [  52]   27 [  49]   24 [  53]   23 [  53]   23 [  23]   56 [  51]   23 [  25]   51 [  53]   23 [  56]   23 [  20]   59 [  18]   53 [  53]   24 [  53]   23 [  24]   52 [  23]   56 [  51]   23 [  53]   23 [  23]   53 [  55]   22 [  22]   54 [  53]   23 [  53]   24 [  22]   54 [  53]   23 [  23]   59 [  48]   23 [  23]   53 [  23]   53 [  23]   54 [  22]   58 [  18]   58 [  52]   21 [  56]   20 [  53]   23 [  23] 20000
//[  24]   50 [  54]   22 [  55]   22 [  54]   22 [  59]   18 [  54]   22 [  54]   23 [  53]   23 [  53]   23 [  54]   22 [  23]   55 [  22]   53 [  53]   23 [  56]   20 [  54]   23 [  23]   53 [  24]   52 [  25]   52 [  53]   23 [  54]   22 [  23]   59 [  17]   54 [  25]   52 [  22]   53 [  54]   23 [  23]   58 [  49]   23 [  57]   19 [  55]   21 [  58]   18 [  23]   55 [  52]   27 [  19]   53 [  57]   19 [  55]   22 [  23]   53 [  23]   53 [  54]   23 [  53]   23 [  23]   53 [  23]   54 [  53]   27 [  52]   20 [  59]   17 [  54]   25 [  21]   58 [  48]   24 [  22]   54 [  22]   54 [  53]   23 [  23]   55 [  52]   27 [  52]   21 [  57]   18 [  23]   54 [  26]   50 [  23]   54 [  53]   23 [  54]   23 [  22]   56 [  20] 22870
//[  24]   50 [  54]   22 [  54]   22 [  54]   23 [  58]   18 [  54]   22 [  54]   23 [  53]   23 [  54]   22 [  57]   23 [  20]   58 [  18]   54 [  53]   23 [  53]   23 [  53]   23 [  23]   54 [  22]   54 [  27]   49 [  53]   23 [  54]   23 [  23]   53 [  23]   53 [  24]   53 [  22]   54 [  55]   21 [  23]   54 [  22]   54 [  56]   20 [  58]   23 [  49]   23 [  23]   53 [  53]   24 [  22]   54 [  56]   24 [  49]   29 [  18]   53 [  23]   53 [  25]   52 [  55]   20 [  53]   24 [  22]   55 [  52]   24 [  52]   24 [  22]   54 [  53]   24 [  22]   54 [  53]   23 [  54]   22 [  56]   23 [  20]   58 [  18]   54 [  23]   54 [  52]   24 [  56]   24 [  18]   59 [  18]   54 [  23]   53 [  22]   54 [  55]   22 [  22]   54 [  22] 19287

// This protocol consists of a bitstream of 60-bits.  The bitstream starts with a "1" (Short-High, Long-Low), followed by 9 times "0" (Long-High,Short-Low). 
// After this header the bitstreams continues with  9 times a "1" and  4-databits (1,2,4,8) .  The 4 databits represent a decimal value which represent the data.
//  The following databit-values "1 3 6 1 2 9 7 3 10" represent a temperature of 21.6c (5,4,3) and humidity of 37,9% (8,7,6).
// The final decimal value seems to be a checksum.
// Long-High, Short-Low is a "0".   Short-High, Long-Low is a "1". Long has twice the duration of Short. A total pulse-cycle (high and low) has 3 times the duration of short.
// The bitstream gets terminated by a short-high and a very long low. 

ELVProtocol::ELVProtocol(
	char * id,
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*TemperatureReceivedEvent)(ProtocolBase * protocol , byte device , float temperature),
	void (*HygroReceivedEvent)(ProtocolBase * protocol , byte device , float hygro)
	) : ConstantLengthPulseCycleProtocolBase(id, BitsstreamReceivedEvent, 60 , 1, (TimerFrequency / 2458.479f) , 1 , 2)
{
	_TemperatureReceivedEvent = TemperatureReceivedEvent;
	_HygroReceivedEvent = HygroReceivedEvent;
}

void ELVProtocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if (decoder_bitpos>=50 && decoder_bitpos<=GetBitstreamLength())
	{
		FlipBits(decoder_bitbuffer,decoder_bitbufferlength , decoder_bitpos );

		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);

		double temp = 0;	
		double hygro = 0;	
		byte checksum = 0;
		bool checksumok = true;
		for (int idx=0;idx<=9;idx++)
		{
			int startbitpos = (decoder_bitpos - 50) + (idx * 5);
			
			// For a valid format, we expect this bit to be set
			if (!GetBit(decoder_bitbuffer,decoder_bitbufferlength, startbitpos)) checksumok = false;
			
			int value = (GetBit(decoder_bitbuffer,decoder_bitbufferlength, startbitpos + 1)?1:0) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,startbitpos+2)?2:0) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,startbitpos+3)?4:0) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,startbitpos+4)?8:0) ;
			if (idx<8) checksum = checksum ^ value;
			switch (idx)
			{
				case 2 : temp += ((double)value / 10.0f); break;
				case 3 : temp += (double)value;break;
				case 4 : temp += ((double)value * 10.0f); break;
				case 5 : hygro += ((double)value / 10.0f); break;
				case 6 : hygro += (double)value;break;
				case 7 : hygro += ((double) value * 10.0f); break;
				case 8 : if (value!=checksum) checksumok = false;
			}			
		}
		if (checksumok)
		{	
			if (_TemperatureReceivedEvent!=0) _TemperatureReceivedEvent(this, 0, temp);
			if (_HygroReceivedEvent!=0) _HygroReceivedEvent(this, 0, hygro);	
		}
	}
}