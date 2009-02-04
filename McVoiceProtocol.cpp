#include "McVoiceProtocol.h"

// Optimization: High bits are shorter ([58,29], [58,28], [57,29])
// Bitstreams from Mc Voice Smoke-detector "FR-1:
// 16 71 16 71 15 71 58 29 14 72 58 28 15 72 57 29 14 72 58 28 15 72 57 29 14 73 56 30 14 73 56 30 56 30 56 31 12 74 13 73 13 74 13 73 14 73 13 73 14 680
// 14 72 14 72 15 72 57 29 14 75 54 30 13 73 57 29 14 74 55 31 13 73 56 30 13 74 56 30 13 73 56 30 56 31 55 31 12 75 12 74 13 74 12 74 13 74 13 74 12 678

// Short0 = 16 , Long0 = 71 , Terminator = 680
// Short1 = 29 , Long1 = 58 
// s0 l0 s0 l0 s0 l0 l1 s1 s0 l0 l1 s1
// PulseCombinations = S0 L0 -> 0, L1, S1 -> 1 , S0, T -> end
// Uses a SE2262 encoder

McVoiceProtocol::McVoiceProtocol(
	char * id,
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*DeviceTrippedEvent)(ProtocolBase * protocol , byte device ),
	void (*DeviceBatteryEmptyEvent)(ProtocolBase * protocol , byte device )
	) : ConstantLengthPulseCycleProtocolBase(id, BitsstreamReceivedEvent, 24 , 4, (TimerFrequency / 11531.3653f) , 3 , 13)
{
	_DeviceTrippedEvent = DeviceTrippedEvent;
	_DeviceBatteryEmptyEvent = DeviceBatteryEmptyEvent;
}

void McVoiceProtocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if (decoder_bitpos==GetBitstreamLength())
	{
		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
	
		bool BatteryEmpty = !GetBit(decoder_bitbuffer, decoder_bitbufferlength, 16);

		unsigned short int device = (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 15)?1:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 13)? 2:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 11)? 4:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 9)?8:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 7)?16:0)  + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 5)? 32:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 3)?64:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 1)?128:0) ;

		if (_DeviceTrippedEvent!=0) _DeviceTrippedEvent(this, device);
		if (_DeviceBatteryEmptyEvent!=0 && BatteryEmpty)  _DeviceBatteryEmptyEvent(this, device);
	}
}

void McVoiceProtocol::EncodeDeviceCommand(byte device, bool lighton, byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;
}