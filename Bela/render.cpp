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
#include <GranularSynth.h>

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
GranularSynth granular;

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
	std::cout<<"begin sleep"<<std::endl;
	sleep(5);
	std::cout<<"end sleep"<<std::endl;
	
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
	monoBuffer.loadSampleFile("vibe.wav");
	stereoBuffer.loadSampleFile("test.wav");
	granular.loadFile("vibe.wav");

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
	const int numAudioFrames = context->audioFrames;
	float mono = 0.0f;
	float left = 0.0f;
	float right = 0.0f;
	float gr[numAudioFrames];
	for(int i = 0; i < numAudioFrames; ++i) {
		gr[i] = 0.0f;
	}
	granular.nextBlock(gr, numAudioFrames);
	
	for(unsigned int i = 0; i < numAudioFrames; ++i)
	{
		monoBuffer.readNext(mono);
		stereoBuffer.readNext(left, right);
		audioWrite(context, i, 0, (gr[i] + mono + left) * 0.1f);
		audioWrite(context, i, 1, (gr[i] + mono + right) * 0.1f);
	}
}

void cleanup(BelaContext *context, void *userData)
{
}
