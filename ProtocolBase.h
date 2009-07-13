#ifndef ProtocolBase_h
#define ProtocolBase_h
#include "DynamicArrayHelper.h"

class ProtocolBase {
	public:
		// Constructor
		ProtocolBase(	
            // An identifying string		
			char * id , 
			
			// An  eventhandler event which will be called when a bitstream has been received and decoded
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer, byte length ) , 
			
			// The length of the bitstream
			int bitstreamlength , 
			
			// The number of repetitions
			int sendrepeats
		);		
		
		// Retrieve the length of the bitstream for this protocol
		int GetBitstreamLength();
		
		// Retrieve the number of repeats needed for this protocol
		int GetSendRepeats();
		
		//  Retrieve a bit from the bitstream buffer
		bool GetBit(byte * bitbuffer, byte bitbufferlength , unsigned short bitpos);
		
		// Retrieve the identifying string
		char * GetId();
		
		// This function returns a buffer with the pulse data of the header for this protocol. 
		virtual void EncodeHeader(unsigned int *& pulsebuffer, byte & pulsebufferlength);

		// This function returns a buffer with the pulse data of a bit-value for this protocol
		virtual void EncodeBit(unsigned int *& pulsebuffer, byte & pulsebufferlength, bool bitvalue);

		// This function returns a buffer with the pulse data of the terminator for this protocol
		virtual void EncodeTerminator(unsigned int *& pulsebuffer, byte & pulsebufferlength);
	private:
	protected:
		// The identifying string of this protocol instance
		char * _id;
		
		// This variable holds the length of the protocol' bistream
		int _bitstreamlength ;
		
		// This variable holds the number of repeats for sending a command for this protocol
		int _sendrepeats ;

		// This variable holds a reference to the decoder bitstream buffer of the stream of bits which have been received
		byte * decoder_bitbuffer ;

		// This variable holds the decoder bitstream buffer bitposition where the next bit will be stored when decoded
		byte decoder_bitpos ;

		// This variable holds the length of the decoder bitstream buffer allocated
		byte decoder_bitbufferlength ;

		// This method will calculate the difference between the value of duration and the expectedduration
		unsigned int CalculateDeviation(unsigned int duration , unsigned int expectedduration);
		
		// This method will return if the duration is within the expected range
		bool WithinExpectedDeviation(unsigned int duration , unsigned int expectedduration , unsigned int acceptabledeviation);
		
		// Store the pulse in a buffer
		void SetPulse(unsigned int *& pulsebuffer, byte & pulsebufferlength, byte pulsepos, unsigned int value);
		
		// Store a bit in a buffer
		void SetBit(byte *& bitbuffer, byte & bitbufferlength, byte bitpos, bool value);
		
		// Add a bit to the buffer
		void AddBit(byte *& bitbuffer, byte & bitbufferlength,  byte& bitpos, bool value);
		
		void SetBitBufferLength(byte *& bitbuffer, byte & bitbufferlength, byte bitpos);
		
		void ShiftFirstBitOut(byte *& bitbuffer, byte & bitbufferlength,  byte& bitpos);
		
		void FlipBits(byte *& bitbuffer, byte & bitbufferlength,  byte bitcount);
		
		// This is a reference to an event which will be invoked when a bitstream is received
		void (*_BitsstreamReceivedEvent)(ProtocolBase * protocol, byte* buffer , byte length);

		// This function resets the decoder state
		virtual void ResetDecoder(void);
		
		DynamicArrayHelper dynamicarrayhelper;
};

#endif
