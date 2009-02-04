#ifndef ELVProtocol_h
#define ELVProtocol_h
#include <ConstantLengthPulseCycleProtocolBase.h>

class ELVProtocol : public ConstantLengthPulseCycleProtocolBase {
	public:
		ELVProtocol (
			char * id,
			double TimerFrequency ,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length),
			void (*TemperatureReceivedEvent)(ProtocolBase * protocol , byte device , float temperature),
			void (*HygroReceivedEvent)(ProtocolBase * protocol , byte device , float hygro)
		);
	private:
		void (*_ValueReceivedEvent)(ProtocolBase * protocol , int value);
	protected:
		void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
		void (*_TemperatureReceivedEvent)(ProtocolBase * protocol, byte device , float temperature);
		void (*_HygroReceivedEvent)(ProtocolBase * protocol, byte device , float hygro);

};

#endif
