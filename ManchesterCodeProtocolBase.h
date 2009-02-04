#ifndef ManchesterCodeProtocolBase_h
#define ManchesterCodeProtocolBase_h
#include <ProtocolBase.h>

class ManchesterCodeProtocolBase : public ProtocolBase {
	public:
		ManchesterCodeProtocolBase(
			char * id,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length),
			int bitstreamlength,
			int sendrepeats,
			float Oscilator,
			float TerminatorOscilators
		);

		void Decode(short state, unsigned int duration);
	private:
	protected:
		unsigned int _highduration;
		float _oscilator;
		float _terminatoroscilators ;
		bool mc_insync;
		virtual void DecodeBitstream();
};

#endif
