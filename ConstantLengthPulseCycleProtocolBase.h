#ifndef ConstantLengthPulseCycleProtocolBase_h
#define ConstantLengthPulseCycleProtocolBase_h
#include <ProtocolBase.h>

class ConstantLengthPulseCycleProtocolBase : public ProtocolBase {
	public:
		ConstantLengthPulseCycleProtocolBase(
			char * id,
			void (*BitsstreamReceivedEvent)( ProtocolBase * protocol , byte* buffer , byte length),
			int bitstreamlength,
			int sendrepeats ,
			float timeperiodduration, 
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
		float _shortperiods ;
		float _longperiods ;
		virtual void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
};

#endif
