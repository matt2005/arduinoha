#ifndef FranElecProtocol_h
#define FranElecProtocol_h
#include <ConstantLengthPulseCycleProtocolBase.h>

class FranElecProtocol : public ConstantLengthPulseCycleProtocolBase {
	public:
		FranElecProtocol(
			char * id, 
			double TimerFrequency ,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
			void (*TrippedEvent)(ProtocolBase * protocol , byte device ),
			void (*BatteryEmptyEvent)(ProtocolBase * protocol, byte device )
		);
	private:
		void (*_TrippedEvent)(ProtocolBase * protocol , byte device );
		void (*_BatteryEmptyEvent)(ProtocolBase * protocol, byte device );
	protected:
		void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);

};

#endif
