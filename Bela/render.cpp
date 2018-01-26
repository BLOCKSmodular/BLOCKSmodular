#include <Bela.h>
#include <Midi.h>
#include <stdlib.h>
#include <cmath>
#include <atomic>
#include <vector>
#include <Smoothing.h>

#define NUMCVOUT 8
#define NUMMORPHLOOPERTRACK 4

enum class ModeList
{
    MicrotonalBlock,
    MorphLooper
};

enum class TransportState
{
    Cleared,
    Stopped,
    Recording,
    Playing
}

int CCInput[NUMCVOUT][2];
Smoothing CVSmooth[NUMCVOUT];
float CVOut[NUMCVOUT] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
ModeList mode = ModeList::MicrotonalBlock;
TransportState morphLooperState[NUMMORPHLOOPERTRACK] = {TransportState::Cleared, TransportState::Cleared, TransportState::Cleared, TransportState::Cleared};
std::vector<int> morphRecording[NUMMORPHLOOPERTRACK];
Midi midi;
const char *gMidiPort0 = "hw:1,0,0";

void midiMessageCallback(MidiChannelMessage message, void *arg)
{
	if (!strcmp(MidiChannelMessage::getTypeText(message.getType()), "control change"))
	{
		int ccNumber = message.getDataByte(0);
		int value = message.getDataByte(1);
		if (96 <= ccNumber && ccNumber < 112)
		{
			int cvOutputChannel = (ccNumber - 96) / 2;
			bool isLowerBit = (ccNumber - 96) % 2;
			CCInput[cvOutputChannel][isLowerBit] = value;
			if (CCInput[cvOutputChannel][0] >= 0 && CCInput[cvOutputChannel][1] >= 0)
			{
				float v = ((CCInput[cvOutputChannel][0] << 7) | (CCInput[cvOutputChannel][1] & 0b1111111)) / 16383.0;
				CVSmooth[cvOutputChannel].set(v);
				CCInput[cvOutputChannel][0] = -1;
				CCInput[cvOutputChannel][1] = -1;
			}
		}
	}
}

bool setup(BelaContext *context, void *userData)
{
	// Set the mode of digital pins
	pinMode(context, 0, P8_08, INPUT);

	for (int i = 0; i < 2; i++)
    {
		for (int k = 0; k < 8; k++) {
			CCInput[i][k] = -1;
		}
	}
    
    for (int i = 0; i < NUMCVOUT; i++) {
        CVSmooth[i].init(1200);
    }
    
    for (int i = 0; i < NUMMORPHLOOPERTRACK; i++) {
        morphRecording[i].reserve(14000);
    }
    
	midi.readFrom(gMidiPort0);
	midi.writeTo(gMidiPort0);
	midi.enableParser(true);
	midi.setParserCallback(midiMessageCallback, (void *)gMidiPort0);
	if (context->analogFrames == 0)
	{
		rt_printf("Error: this example needs the analog I/O to be enabled\n");
		return false;
	}

	if (context->audioOutChannels < 2 || context->analogOutChannels < 2)
	{
		printf("Error: for this project, you need at least 2 analog and audio output channels.\n");
		return false;
	}

	return true;
}

void render(BelaContext *context, void *userData)
{
    //チャタリングを防止するコードに直すこと!!
	int status = digitalRead(context, 0, P8_08);
	if (status == 1)
	{
        mode = ModeList::MorphLooper
	}else{
        mode = ModeList::MicrotonalBlock
	}
    
    midi_byte_t bytes[3] = {176, (midi_byte_t)(mode == ModeList::MorphLooper ? 96 : 97), 127};
	midi.writeOutput(bytes, 3);

	for (unsigned int n = 0; n < context->analogFrames; n++)
	{
		for (unsigned int i = 0; i < NUMCVOUT; i++)
		{
			CVOut[i] = CVSmooth[i].getNextValue();
			analogWrite(context, n, i, CVOut[i]);
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{
}
