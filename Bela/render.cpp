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

static constexpr int NUMCVOUT = 8;
static constexpr unsigned char CVModeONE =   0b00000001;
static constexpr unsigned char CVModeTWO =   0b00000010;
static constexpr unsigned char CVModeOFF =   0b00000100;
static constexpr unsigned char CVModeTHREE = 0b00001000;
static constexpr unsigned char CVModeFOUR =  0b00010000;
static constexpr unsigned char AudioModeONE =   0b00000001;
static constexpr unsigned char AudioModeTWO =   0b00000010;
static constexpr unsigned char AudioModeOFF =   0b00000100;
static constexpr unsigned char AudioModeTHREE = 0b00001000;
static constexpr unsigned char AudioModeFOUR =  0b00010000;
unsigned char CVmodeFlag;
unsigned char AudiomodeFlag;
Midi midi;
const char *gMidiPort0 = "hw:1,0,0";
MonoBuffer monoBuffer(88200, true, false);
StereoBuffer stereoBuffer(88200, true, false);
GranularSynth granular;

void midiMessageCallback(MidiChannelMessage message, void *arg)
{
	if (!strcmp(MidiChannelMessage::getTypeText(message.getType()), "control change"))
	{
		const int ccNumber = message.getDataByte(0);
		const int value = message.getDataByte(1);
	}
}

bool setup(BelaContext *context, void *userData)
{
	std::cout<<"begin sleep"<<std::endl;
	sleep(5);
	std::cout<<"end sleep"<<std::endl;
	
	CVmodeFlag = CVModeONE;
	AudiomodeFlag = AudioModeONE;
	
	//Set the mode of digital pins
	//Audio Mode
	pinMode(context, 0, P8_07, INPUT);
	pinMode(context, 0, P8_08, INPUT);
	pinMode(context, 0, P8_09, INPUT);
	pinMode(context, 0, P8_10, INPUT);
	pinMode(context, 0, P8_11, INPUT);
	//CV Mode
	pinMode(context, 0, P8_18, INPUT);
	pinMode(context, 0, P8_27, INPUT);
	pinMode(context, 0, P8_28, INPUT);
	pinMode(context, 0, P8_29, INPUT);
	pinMode(context, 0, P8_30, INPUT);
    
    //MIDI
	midi.readFrom(gMidiPort0);
	midi.writeTo(gMidiPort0);
	midi.enableParser(true);
	midi.setParserCallback(midiMessageCallback, (void *)gMidiPort0);
	
	midi_byte_t bytes[3] = {176, (midi_byte_t)(97), 127};
	midi.writeOutput(bytes, 3);
	
	//Load Sample
	monoBuffer.loadSampleFile("vibe.wav");
	stereoBuffer.loadSampleFile("test.wav");
	granular.loadFile("vibe.wav");

	return true;
}

void render(BelaContext *context, void *userData)
{
	unsigned char cvFLG = 0;
	unsigned char audioFLG = 0;
	//TODOチャタリング除去
	if(digitalRead(context, 0, P8_07)) cvFLG = CVModeONE;
	if(digitalRead(context, 0, P8_08)) cvFLG = CVModeTWO;
	if(digitalRead(context, 0, P8_09)) cvFLG = CVModeOFF;//P8_09はOFFスイッチ
	if(digitalRead(context, 0, P8_10)) cvFLG = CVModeTHREE;
	if(digitalRead(context, 0, P8_11)) cvFLG = CVModeFOUR;
	if(CVmodeFlag != cvFLG && cvFLG != 0)
	{
		//TODO CVModeリセット関数を追加
		//sendMIDItoCVModeReset();
		
		if(cvFLG == CVModeONE) {
			//CVmode1
			//モード切り替え用CC送信
			//midi_byte_t bytes[3] = {176, (midi_byte_t)(97), 127};
			//midi.writeOutput(bytes, 3);
		}
		else if(cvFLG == CVModeTWO) {
			//CVmode2
			//モード切り替え用CC送信
		}
		else if(cvFLG == CVModeOFF) {
			//CVmode OFF
			//モード切り替え用CC送信
		}
		else if(cvFLG == CVModeTHREE) {
			//CVmode3
			//モード切り替え用CC送信
		}
		else if(cvFLG == CVModeFOUR) {
			//CVmode4
			//モード切り替え用CC送信
		}
		CVmodeFlag = cvFLG;
	}
	
	if(digitalRead(context, 0, P8_18)) audioFLG = AudioModeONE;
	if(digitalRead(context, 0, P8_27)) audioFLG = AudioModeTWO;
	if(digitalRead(context, 0, P8_28)) audioFLG = AudioModeOFF;//P8_28はOFFスイッチ
	if(digitalRead(context, 0, P8_29)) audioFLG = AudioModeTHREE;
	if(digitalRead(context, 0, P8_30)) audioFLG = AudioModeFOUR;
	if(AudiomodeFlag != audioFLG && audioFLG != 0)
	{
		//TODO audioModeリセット関数を追加
		//sendMIDItoAudioModeReset();
		
		if(audioFLG == AudioModeONE) {
			//Audiomode1
			//モード切り替え用CC送信
			//midi_byte_t bytes[3] = {176, (midi_byte_t)(97), 127};
			//midi.writeOutput(bytes, 3);
		}
		else if(audioFLG == AudioModeTWO) {
			//Audiomode2
			//モード切り替え用CC送信
		}
		else if(audioFLG == AudioModeOFF) {
			//Audiomode OFF
			//モード切り替え用CC送信
		}
		else if(audioFLG == AudioModeTHREE) {
			//Audiomode3
			//モード切り替え用CC送信
		}
		else if(audioFLG == AudioModeFOUR) {
			//Audiomode4
			//モード切り替え用CC送信
		}
		AudiomodeFlag = audioFLG;
	}


	switch(CVmodeFlag)
	{
		case CVModeONE:
		{
			break;
		}
		case CVModeTWO:
		{
			break;
		}
		case CVModeOFF:
		{
			break;
		}
		case CVModeTHREE:
		{
			break;
		}
		case CVModeFOUR:
		{
			break;
		}
		default:
		{
			rt_printf("CVMode: %d\n", CVmodeFlag);
			break;
		}
	}
	
	const int numAudioFrames = context->audioFrames;
	switch(AudiomodeFlag)
	{
		case AudioModeONE:
		{
			//Granular
			float gr[numAudioFrames];
			for(unsigned int i = 0; i < numAudioFrames; ++i) {
				gr[i] = 0.0f;
			}
			granular.nextBlock(gr, numAudioFrames);
	
			for(unsigned int i = 0; i < numAudioFrames; ++i) {
				audioWrite(context, i, 0, gr[i] * 0.2f);
				audioWrite(context, i, 1, gr[i] * 0.2f);
			}
			break;
		}
		case AudioModeTWO:
		{
			//MonoBuffer
			float mono = 0.0f;
			for(unsigned int i = 0; i < numAudioFrames; ++i) {
				monoBuffer.readNext(mono);
				audioWrite(context, i, 0, mono);
				audioWrite(context, i, 1, mono);
			}
			break;
		}
		case AudioModeOFF:
		{
			break;
		}
		case AudioModeTHREE:
		{
			//StereoBuffer
			float stereo[2] = {0.0f, 0.0f};
			for(unsigned int i = 0; i < numAudioFrames; ++i) {
				stereoBuffer.readNext(stereo[0], stereo[1]);
				audioWrite(context, i, 0, stereo[0]);
				audioWrite(context, i, 1, stereo[1]);
			}
			break;
		}
		case AudioModeFOUR:
		{
			break;
		}
		default:
		{
			rt_printf("AudioMode: %d\n", AudiomodeFlag);
			break;
		}
	}
	
	
	// // CV/Gate
	// for (unsigned int n = 0; n < context->analogFrames; n++)
	// {
	// 	for (unsigned int i = 0; i < NUMCVOUT; i++)
	// 	{
	// 		// float v = CVSmooth[i].getNextValue();
	// 		// analogWrite(context, n, i, v);
	// 	}
	// }

	// // Audio
	// const int numAudioFrames = context->audioFrames;
	// float mono = 0.0f;
	// float left = 0.0f;
	// float right = 0.0f;
	// float gr[numAudioFrames];
	// for(int i = 0; i < numAudioFrames; ++i) {
	// 	gr[i] = 0.0f;
	// }
	// granular.nextBlock(gr, numAudioFrames);
	
	// for(unsigned int i = 0; i < numAudioFrames; ++i)
	// {
	// 	monoBuffer.readNext(mono);
	// 	stereoBuffer.readNext(left, right);
	// 	audioWrite(context, i, 0, (gr[i] + mono + left) * 0.2f);
	// 	audioWrite(context, i, 1, (gr[i] + mono + right) * 0.2f);
	// }
}

void cleanup(BelaContext *context, void *userData)
{
}
