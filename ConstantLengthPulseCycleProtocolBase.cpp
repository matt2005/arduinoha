#include "ConstantLengthPulseCycleProtocolBase.h"
#include <hardwareserial.h>

// This class is a decoder/encoder which should be compatible with the PT2262 encoder chip from Princeton.
// This class is based on this datasheet: http://www.princeton.com.tw/downloadprocess/downloadfile.asp?mydownload=PT2262_1.pdf
// The PT2262 is a tristate encoder by which two "0" pulse-cycles mean a "0"-bit, two "1" pulse-cycles mean a "1"-bit. A "0" pulse-cycle followed by a "1" pulse-cycle mean bit "f" (floating bit)

// The PT2262 encoded signal consits of a constant time length pulse cycle. The duration of the Highpulse & Lowpulse are everytime equal long.
// The PT2262 encoded signal ends with a terminator (aka sync-bit-cycle)

#define HIGH 0x1
#define LOW  0x0


ConstantLengthPulseCycleProtocolBase::ConstantLengthPulseCycleProtocolBase(
	char * id,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),	
	int bitstreamlength,
	int sendrepeats , 
	float timeperiodduration , // The duration of one oscillation
	float shortperiods , // The number of timeperiods in a short-pulse
	float longperiods // The number of timeperiods in a long-pulse
	) : ProtocolBase(id , BitsstreamReceivedEvent , bitstreamlength , sendrepeats)
{
	_timeperiodduration = timeperiodduration;
	_shortperiods = shortperiods;
	_longperiods = longperiods;
	
	_highpulseduration = 0;
}

void ConstantLengthPulseCycleProtocolBase::Decode(short state, unsigned int duration)
{
    if (state==LOW)
    {
      if (WithinExpectedDeviation( duration + _highpulseduration , (_shortperiods+_longperiods) * _timeperiodduration ,  _timeperiodduration / 2 ) )
      {
	  	// Are there more bits in the buffer than is expected for this protocol?
		if (decoder_bitpos == GetBitstreamLength()) 
		{ 
			ShiftFirstBitOut(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
        }

        if (_highpulseduration > duration )
        { // Long High, Short  Low == 1
          AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true);
        } else
        { // Short High, Long Low == 0
          AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);
        }		
      } else 
	  {		
		DecodeBitstream( _highpulseduration , duration);
        ResetDecoder();
	  }
    } else if (state==HIGH)
	{
		_highpulseduration = duration;
	}
}

// This function returns a buffer with the pulse data of a bit for this protocol
void ConstantLengthPulseCycleProtocolBase::EncodeBit(unsigned int *& pulsebuffer, byte & pulsebufferlength, bool bitvalue)
{
  pulsebuffer = 0;
  pulsebufferlength = 0;
  if (bitvalue)
  {
    SetPulse(pulsebuffer, pulsebufferlength, 0 , _longperiods * _timeperiodduration );
    SetPulse(pulsebuffer, pulsebufferlength, 1 , _shortperiods * _timeperiodduration );
  } else
  {
    SetPulse(pulsebuffer, pulsebufferlength, 0 , _shortperiods * _timeperiodduration );
    SetPulse(pulsebuffer, pulsebufferlength, 1 , _longperiods * _timeperiodduration );
  }
}

// This function returns a buffer with the pulse data of the terminator for this protocol
void ConstantLengthPulseCycleProtocolBase::EncodeTerminator(unsigned int *& pulsebuffer, byte & pulsebufferlength) 
{
  pulsebuffer = 0;
  pulsebufferlength = 0;
  
  SetPulse(pulsebuffer, pulsebufferlength, 0 , _shortperiods * _timeperiodduration );
  SetPulse(pulsebuffer, pulsebufferlength, 1 , 32 * _timeperiodduration );
}




void ConstantLengthPulseCycleProtocolBase::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
}