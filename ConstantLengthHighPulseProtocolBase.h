#ifndef ConstantLengthHighPulseProtocolBase_h
#define ConstantLengthHighPulseProtocolBase_h
#include <ProtocolBase.h>

class ConstantLengthHighPulseProtocolBase : public ProtocolBase {
	public:
		ConstantLengthHighPulseProtocolBase(
			char * id,
			void (*BitsstreamReceivedEvent)( ProtocolBase * protocol , byte* buffer , byte length),
			int bitstreamlength,
			int sendrepeats ,
			float timeperiodduration,
			float syncbitperiods , 
			float shortperiods ,
			float longperiods 
		);
		
		virtual void EncodeBit(unsigned int *& pulsebuffer, byte & pulsebufferlength, bool bitvalue);
		virtual void EncodeTerminator(unsigned int *& pulsebuffer, byte & pulsebufferlength);

		void Decode(short state, unsigned int duration);
	private:
		unsigned int _highpulseduration;
	protected:
		float _timeperiodduration;	
		float _syncbitperiods ; 
		float _shortperiods ;
		float _longperiods ;
		virtual void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
};

#endif
