#ifndef SkytronicProtocol_h
#define SkytronicProtocol_h
#include <ConstantLengthDoublePulseCycleProtocolBase.h>

class SkytronicProtocol : public ConstantLengthDoublePulseCycleProtocolBase {
	public:
		SkytronicProtocol(
			char * id,
			double TimerFrequency ,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length),
			void (*DeviceCommandReceivedEvent)(ProtocolBase * protocol , byte device , bool turnoncommand),
			void (*AllDevicesCommandReceivedEvent)(ProtocolBase * protocol , bool turnoncommand)
		);

		void EncodeDeviceCommand(byte device, bool lighton, byte *& bitbuffer, byte &bitbufferlength );
		void EncodeAllDevicesCommand(bool lighton, byte *& bitbuffer, byte &bitbufferlength );
	private:
		void (*_DeviceCommandReceivedEvent)(ProtocolBase * protocol , byte device , bool turnoncommand);
		void (*_AllDevicesCommandReceivedEvent)(ProtocolBase * protocol , bool turnoncommand);
	protected:
		void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
};

#endif
