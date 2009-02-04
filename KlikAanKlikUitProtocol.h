#ifndef KlikAanKlikUitProtocol_h
#define KlikAanKlikUitProtocol_h
#include <ConstantLengthPulseCycleProtocolBase.h>

class KlikAanKlikUitProtocol : public ConstantLengthPulseCycleProtocolBase {
	public:
		KlikAanKlikUitProtocol (
			char * id,
			double TimerFrequency,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length),
			void (*DeviceCommandReceivedEvent)(ProtocolBase * protocol , byte device , bool turnoncommand)
		);

		void EncodeDeviceCommand(byte device, bool lighton, byte *& encodedpulsestream, byte &encodedpulsestreamlength );
	private:
		void (*_DeviceCommandReceivedEvent)(ProtocolBase * protocol, byte device , bool turnoncommand);
	protected:
		void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
};

#endif
