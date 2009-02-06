#include "ProtocolBase.h"
#include <stdlib.h>

/*!
 * \brief Pure-virtual workaround.
 *
 * The avr-libc does not support a default implementation for handling 
 * possible pure-virtual calls. This is a short and empty workaround for this.
 */
extern "C" {
  void __cxa_pure_virtual()
  {
    // put error handling here
  }
}


ProtocolBase::ProtocolBase(
	char * id,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer, byte length ) ,
	int bitstreamlength,
	int sendrepeats
)
{
	_id = id;	
	_BitsstreamReceivedEvent = BitsstreamReceivedEvent;	
	_bitstreamlength = bitstreamlength;
	_sendrepeats = sendrepeats;	

	decoder_bitbuffer = 0;
	
	ResetDecoder();
}

// This method return the length of the normal bitstream
int ProtocolBase::GetBitstreamLength()
{
	return _bitstreamlength;
}

// This method returns the number of repeats te protocol normally is sent
int ProtocolBase::GetSendRepeats()
{
	return _sendrepeats;
}

// This function resets the decoder state
void ProtocolBase::ResetDecoder(void)
{
  if (decoder_bitbuffer!=0) 
  {
    free(decoder_bitbuffer);
    decoder_bitbuffer = 0;
  }

  decoder_bitpos = 0;
  decoder_bitbufferlength = 0;
}



unsigned int ProtocolBase::CalculateDeviation(unsigned int duration , unsigned int expectedduration)
{
  long deviation = (long)duration ;
  deviation -= (long)expectedduration;
  deviation = (deviation<0?-deviation:deviation);
  return deviation;
}

bool ProtocolBase::WithinExpectedDeviation(unsigned int duration , unsigned int expectedduration , unsigned int acceptabledeviation)
{
    if (CalculateDeviation(duration, expectedduration)<=acceptabledeviation) return true;
    return false;
}

void ProtocolBase::SetPulse(unsigned int *& pulsebuffer, byte & pulsebufferlength, byte pulsepos, unsigned int value)
{  
  dynamicarrayhelper.SetElementInArray( (void *&)pulsebuffer, &value , pulsepos , pulsebufferlength ,  sizeof(unsigned int) );
}

// This function sets a bit in a byte-buffer.
void ProtocolBase::SetBit(byte *& bitbuffer, byte & bitbufferlength, byte bitpos, bool value)
{
  // Calculate at what byte-position the bit needs to be set or cleared in the byte.
  int bytepos = bitpos >> 3; // Divide by 8, because 8 bits are stored in one byte
  //  Is the position outside the current allocated bitbuffer ?
  if (bytepos>=bitbufferlength)
  { // Yes, it's outside the current allocated bitbuffer
	// Increase the size of the bitbuffer to be able to store the bit in the byte.
	dynamicarrayhelper.SetArrayLength( (void *&)bitbuffer , bytepos + 1 , bitbufferlength , sizeof(byte) );
  }
  
  // Is the position inside the current allocated bitbuffer ?
  if (bytepos<bitbufferlength)
  { // Yes
    byte bitvalue = (128 >> (bitpos % 8));
	if (value)
	{ // Set the bit
		bitbuffer[ bytepos ] |= bitvalue;
	} else
	{ // Clear the bit
		bitbuffer[ bytepos ] &= (~ bitvalue );
	}
  }
}

// This function sets a bit in the buffer and increments the bitpos
void ProtocolBase::AddBit(byte *& bitbuffer, byte & bitbufferlength,  byte& bitpos, bool value)
{
  SetBit(bitbuffer, bitbufferlength, bitpos, value);
  bitpos++;
}

// This function shifts the first bit out of the buffer
void ProtocolBase::ShiftFirstBitOut(byte *& bitbuffer, byte & bitbufferlength,  byte& bitpos)
{
  for (int idx=1;idx<bitpos;idx++)
  {
	SetBit(bitbuffer , bitbufferlength, idx - 1 , GetBit( bitbuffer , bitbufferlength , idx ) );
  }
  bitpos --;  
}

// This function returns the bitvalue
bool ProtocolBase::GetBit(byte * bitbuffer, byte bitbufferlength , unsigned short bitpos)
{
  // Calculate from what byte-position the bit needs to be retrieved.
  byte bytepos = bitpos >> 3;
  
  // Is the bit outside the buffer?
  if (bytepos >= bitbufferlength || bitbuffer==0) 
  { // bitpos is outside the allocated buffer range
    return false;
  }
  
  if ((bitbuffer[bytepos] & (128 >> (bitpos % 8))) != 0) return true;
  return false;
}

//
void ProtocolBase::SetBitBufferLength(byte *& bitbuffer, byte & bitbufferlength, byte bitpos)
{
	SetBit(bitbuffer, bitbufferlength , bitpos , false);
}

// This method flips inverts the bits of the buffer
void ProtocolBase::FlipBits(byte *& bitbuffer, byte & bitbufferlength, byte bitcount)
{
	for (int idx=0;idx<bitcount;idx++)
	{
		SetBit( bitbuffer , bitbufferlength, idx , !GetBit( bitbuffer , bitbufferlength, idx));
	}
}

