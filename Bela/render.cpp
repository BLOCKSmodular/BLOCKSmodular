#include <Bela.h>
#include <Midi.h>
#include <stdlib.h>
#include <cmath>
#include <atomic>
#include <vector>
#include <random>
#include <Smoothing.h>
#include <SampleBuffer.h>
#include <Util.h>

#define NUMCVOUT 8
#define NUMMORPHLOOPERTRACK 2

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
};

int CCInput[NUMCVOUT][2];
Smoothing CVSmooth[NUMCVOUT];
ModeList mode = ModeList::MicrotonalBlock;
TransportState morphLooperState[NUMMORPHLOOPERTRACK] = {TransportState::Cleared, TransportState::Cleared};
int lastSwitchState = 0;
Midi midi;
const char *gMidiPort0 = "hw:1,0,0";
MonoBuffer monoBuffer(88200, true, false);
StereoBuffer stereoBuffer(88200, true, false);

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
	// // Set the mode of digital pins
	// pinMode(context, 0, P8_07, INPUT);
	// pinMode(context, 0, P8_08, INPUT);
	
	//Init smoothing class
    for (int i = 0; i < NUMCVOUT; i++) {
        CVSmooth[i].init(1200);
    }

	for (int i = 0; i < 2; i++)
    {
		for (int k = 0; k < 8; k++) {
			CCInput[i][k] = -1;
		}
	}
    
    //MIDI
	midi.readFrom(gMidiPort0);
	midi.writeTo(gMidiPort0);
	midi.enableParser(true);
	midi.setParserCallback(midiMessageCallback, (void *)gMidiPort0);
	
	//Load Sample
	std::string wavFile("vibe.wav");
	monoBuffer.loadSampleFile(wavFile);
	stereoBuffer.loadSampleFile("test.wav");
	// const float* hoge = stereoBuffer.getReadChannelPtr(0);
	// for(int i = 0 ; i < stereoBuffer.getSize(); ++i)
	// {
	// 	std::cout<<hoge[i]<<", ";
	// }

	return true;
}

void render(BelaContext *context, void *userData)
{
	// for (unsigned int n = 0; n < context->digitalFrames; n++)
	// {
	// 	//TODO チャタリング除去
	// 	int swc = digitalRead(context, n, P8_08);
	//  	if(lastSwitchState != swc)//モード切り替えスイッチ
 //		{
 //			mode = swc == 0 ? ModeList::MorphLooper : ModeList::MicrotonalBlock;
 //			lastSwitchState = swc;
 //			midi_byte_t bytes[3] = {176, (midi_byte_t)(mode == ModeList::MorphLooper ? 96 : 97), 127};
	// 		midi.writeOutput(bytes, 3);
 //		}
 		
 //		if(digitalRead(context, 0, P8_07) == 0)//録音クリアースイッチ
 //		{
 //			for (int i = 0; i < 4; i++) 
 //			{
 //				midi_byte_t bytes[3] = {176, (midi_byte_t)(72 + i), 127};
	// 			midi.writeOutput(bytes, 3);
 //			}
 //		}
	// }

	// CV/Gate
	for (unsigned int n = 0; n < context->analogFrames; n++)
	{
		for (unsigned int i = 0; i < NUMCVOUT; i++)
		{
			float v = CVSmooth[i].getNextValue();
			analogWrite(context, n, i, v);
		}
	}
	
	// Audio
	float l = 0.0f;
	float r = 0.0f;
	for (unsigned int n = 0; n < context->audioFrames; n++)
	{
		float v = *monoBuffer.readNext();
		stereoBuffer.readNext(l, r);
		// rt_printf("%f\n", l);
		audioWrite(context, n, 0, (v + l) * 0.5f);
		audioWrite(context, n, 1, (v + r) * 0.5f);
	}
}

void cleanup(BelaContext *context, void *userData)
{
}
