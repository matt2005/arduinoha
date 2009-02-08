#include "FranElecProtocol.h"

// Product: 	PIR Motion Sensor
// Productcode: FRANELEC SENS01
// Chip: 	PT2262
// [  26] 91 [  81] 32 [  25] 92 [  80] 33 [  24] 94 [  79] 34 [  23] 94 [  79] 35 [  23] 94 [  79] 35 [  22] 95 [  79] 36 [  21] 96 [  78] 36 [  22] 96  [  77] 38 [  76] 38 [  77] 38 [  20] 98 [  20] 97 [  21] 97 [  21] 97 [  20] 97 [  21] 97 [  21] 933
// [  31] 89 [  84] 29 [  28] 88 [  87] 27 [  31] 88 [  28] 89 [  29] 91 [  84] 27 [  32] 87 [  29] 88 [  30] 88 [  29] 91 [  27] 89 [  29] 88 [  31] 87 [  30] 89 [  85] 32 [  82] 34 [  26] 88 [  30] 89 [  29] 88 [  30] 91 [  26] 92 [  26] 91 [  28]  925 

//[  12]   76 [  67]   20 [  26]   84 [  62]   24 [  24]   85 [  23]   86 [  23]   86 [  56]   27 [  21]   88 [  56]   28 [  20]   88 [  56]   28 [  20]   90 [  18]   90 [  19]   89 [  20]   88 [  20]   88 [  54]   28 [  19]   90 [  19]   89 [  19]   88 [  21]   87 [  20]   88 [  21]   88 [  20]  936

// Product:	Wireless Door Bell
// Productcode: FRANELEC ANTI01
// Chip:	M3D-95 / E0927

#define MaxDeviation 15
#define CombinationDurationLong 113
#define CombinationDurationShort 92
#define MinTerminatorDuration 800
#define Terminator 933

FranElecProtocol::FranElecProtocol(
	char * id, 
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*TrippedEvent)(ProtocolBase * protocol , byte device ),
	void (*BatteryEmptyEvent)(ProtocolBase * protocol , byte device ) ) : ConstantLengthPulseCycleProtocolBase(id, BitsstreamReceivedEvent , 24 , 4 , (TimerFrequency / 2136.75f) , 1 , 3 )
{
	_TrippedEvent = TrippedEvent;
	_BatteryEmptyEvent = BatteryEmptyEvent;
}

void FranElecProtocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{	
	if (decoder_bitpos==GetBitstreamLength())
	{
		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
		bool BatteryEmpty = !(GetBit(decoder_bitbuffer,decoder_bitbufferlength,16));
	
		byte device = 0;
		for (int idx=0; idx <= 7;idx++)
		{
			device |= (GetBit(decoder_bitbuffer,decoder_bitbufferlength,15 - (idx *2) )?0:1 << idx) ;
		}
		
		bool valid = true;
		for (int idx=0; idx<=7;idx++)
		{
			if (GetBit(decoder_bitbuffer,decoder_bitbufferlength,(idx*2))) valid = false;
		}
		
		for (int idx=18;idx<=22;idx++)
		{
			if (GetBit(decoder_bitbuffer,decoder_bitbufferlength,idx)) valid = false;
		}

		if (valid)
		{
			if (_TrippedEvent!=0) _TrippedEvent( this , device);
			if (_BatteryEmptyEvent!=0 && BatteryEmpty)  _BatteryEmptyEvent( this , device);
		}
	}
}

/*void FranElecProtocol::Decode(short state, unsigned int duration)
{
    if (state==LOW)
    {
      if (WithinExpectedDeviation( duration + prevduration , CombinationDurationLong ,  MaxDeviation) ||
		WithinExpectedDeviation( duration + prevduration , CombinationDurationShort ,  MaxDeviation)  )
      {
        if (prevduration > duration )
        { // 0
          AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);
        } else
        { // 1
          AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true);
        }
		if ( decoder_bitpos>GetBitstreamLength() ) ResetDecoder();
      } else
      {
        if (decoder_bitpos==24 && duration > MinTerminatorDuration )
        { 
			if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
			bool BatteryEmpty = !(GetBit(decoder_bitbuffer,decoder_bitbufferlength,16));

			unsigned short int device = (GetBit(decoder_bitbuffer,decoder_bitbufferlength,15)?0:1) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,13)?0:2) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,11)?0:4) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,9)?0:8) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,7)?0:16) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,5)?0:32) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,3)?0:64) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,1)?0:128) ;

			if (_TrippedEvent!=0) _TrippedEvent( this , device);
			if (_BatteryEmptyEvent!=0 && BatteryEmpty)  _BatteryEmptyEvent( this , device);
        }
		ResetDecoder();
      } 
    }
    prevduration = duration;
}
*/