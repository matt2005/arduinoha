#ifndef ConstantLengthLowPulseProtocolBase_h
#define ConstantLengthLowPulseProtocolBase_h
#include <ProtocolBase.h>

class ConstantLengthLowPulseProtocolBase : public ProtocolBase {
	public:
		ConstantLengthLowPulseProtocolBase(
			char * id,
			void (*BitsstreamReceivedEvent)( ProtocolBase * protocol , byte* buffer , byte length),
			int bitstreamlength,
			int sendrepeats ,
			float timeperiodduration,
			float shortperiods ,
			float longperiods 
		);
		
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
