#ifndef LaCrosseProtocol_h
#define LaCrosseProtocol_h
#include <ProtocolBase.h>

class LaCrosseProtocol : public ProtocolBase {
	public:
		LaCrosseProtocol(
			// ProtocolBase: An identifying string		
			char * id,
			double TimerFrequency ,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length),
			void (*TemperatureReceivedEvent)(ProtocolBase * protocol , byte device , float temperature),
			void (*HygroReceivedEvent)(ProtocolBase * protocol , byte device , float hygro),
			void (*RainReceivedEvent)(ProtocolBase * protocol, byte device , int rain)
		);

		void Decode(short state, unsigned int duration);
		
		void EncodeBit(unsigned int *& pulsebuffer, byte & pulsebufferlength, bool bitvalue);
		
		void EncodeTerminator(unsigned int *& pulsebuffer, byte & pulsebufferlength) ;
		
		// This function returns a bitbuffer
		void EncodeTemperatureCommand(byte device, float value , byte *& bitbuffer, byte &bitbufferlength );
		void EncodeRainCommand(byte device, int value , byte *& bitbuffer, byte &bitbufferlength );
		void EncodeHygroCommand(byte device, float value , byte *& bitbuffer, byte &bitbufferlength );
	private:
		unsigned int prevduration;
		float _timeperiodduration;
		float _maxdeviation;
		void (*_TemperatureReceivedEvent)(ProtocolBase * protocol, byte device , float temperature);
		void (*_HygroReceivedEvent)(ProtocolBase * protocol, byte device , float hygro);
		void (*_RainReceivedEvent)(ProtocolBase * protocol, byte device , int rain);
	protected:
};

#endif
