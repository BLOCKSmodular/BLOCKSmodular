#include <Bela.h>
#include <Midi.h>
#include <stdlib.h>
#include <cmath>
#include <atomic>
#include <vector>
#include <random>
#include <SampleBuffer.h>
#include <Util.h>
#include <GranularSynth.h>

static constexpr int NUMCVOUT = 8;
static constexpr int NUMSAMPLEPLAYBUFFER = 4;
static constexpr unsigned char CVModeA =   0b00000001;
static constexpr unsigned char CVModeB =   0b00000010;
static constexpr unsigned char CVModeC =   0b00001000;
static constexpr unsigned char CVModeD =   0b00010000;
static constexpr unsigned char AudioModeA =   0b00000001;
static constexpr unsigned char AudioModeB =   0b00000010;
static constexpr unsigned char AudioModeC =   0b00001000;
static constexpr unsigned char AudioModeD =   0b00010000;
unsigned char CVmodeFlag = 0;
unsigned char AudiomodeFlag = 0;
bool isAudioPage = false;
Midi midi;
const char *gMidiPort0 = "hw:1,0,0";

GranularSynth granular;
HighResolutionControlChange gr_Position[2];
HighResolutionControlChange gr_GrainSize[2];
HighResolutionControlChange gr_WindowShape[2];
HighResolutionControlChange microtone_Distance[4];
HighResolutionControlChange microtone_Pressure[4];
StereoBuffer* samplePlay_buffer;
bool samplePlay_isPlaying[4]{false, false, false, false};
Smoothing CVSmooth[NUMCVOUT];
Smoothing outputGain;


void midiMessageCallback(MidiChannelMessage message, void *arg)
{
	const int channel = message.getChannel();//MIDIChannel 0~15
	
	//Note On
	if(message.getType() == kmmNoteOn){
	}

	//Control change
    if(message.getType() == kmmControlChange)
    {
        const int controlNum = message.getDataByte(0);
        const int value = message.getDataByte(1);
        
        if(channel == 15) {
            //General messeges
        }
        else if(channel < 8) {
            //Audio
            const int voiceIndex = channel;
            switch(AudiomodeFlag) {
                case AudioModeA: {
                    //Granular
                    if(controlNum == 1 || controlNum == 2) {
                        bool isUpeerByte{controlNum == 1};
                        gr_Position[voiceIndex].set(value, isUpeerByte);
                        if(gr_Position[voiceIndex].update()) granular.setBufferPosition(gr_Position[voiceIndex].get(), voiceIndex);
                    }
                    
                    if(controlNum == 3 || controlNum == 4) {
                        bool isUpeerByte{controlNum == 3};
                        gr_GrainSize[voiceIndex].set(value, isUpeerByte);
                        if(gr_GrainSize[voiceIndex].update()) granular.setGrainSize(gr_GrainSize[voiceIndex].get(), voiceIndex);
                    }
                    
                    if(controlNum == 5 || controlNum == 6) {
                        bool isUpeerByte{controlNum == 5};
                        gr_WindowShape[voiceIndex].set(value, isUpeerByte);
                        if(gr_WindowShape[voiceIndex].update()) {
                            float v = gr_WindowShape[voiceIndex].get() * 2.0f;
                            granular.setWindowShape(v * 4.0f, voiceIndex);
                            granular.setDensity((v * v * v), voiceIndex);
                        }
                    }
                    break;
                }
                case AudioModeB: {
                    //Sample playback
                    if(controlNum == 1) {
                    	if(value == 127) {
                    		samplePlay_isPlaying[voiceIndex] = true;
                    	}
                    }
                    break;
                }
                case AudioModeC: {
                    //Karplus strong
                    break;
                }
                case AudioModeD: {
                    //Logistic map
                    break;
                }
                default: {
                    rt_printf("AudioMode: %d\n", AudiomodeFlag);
                    break;
                }
            }
        }
        else {
            const int voiceIndex = channel - 8;
            //CV
            switch(CVmodeFlag) {
                case CVModeA: {
                    //Morph looper
                    break;
                }
                case CVModeB: {
                    //Microtonal
                	if(voiceIndex >= 4) {
                		std::cout<<"MIDI: Invalid voice number"<<std::endl;
                		break;
                	}
                	
                    if(controlNum == 1 || controlNum == 2) {
                        bool isUpeerByte{controlNum == 1};
                        microtone_Distance[voiceIndex].set(value, isUpeerByte);
                        if(microtone_Distance[voiceIndex].update()) CVSmooth[voiceIndex * 2].set(microtone_Distance[voiceIndex].get());
                    }
                    
                    if(controlNum == 3 || controlNum == 4) {
                        bool isUpeerByte{controlNum == 3};
                        microtone_Pressure[voiceIndex].set(value, isUpeerByte);
                        if(microtone_Pressure[voiceIndex].update()) CVSmooth[voiceIndex * 2 + 1].set(microtone_Pressure[voiceIndex].get());
                    }
                    break;
                }
                case CVModeC: {
                    break;
                }
                case CVModeD: {
                    //Euclid sequence
                    break;
                }
                default: {
                    rt_printf("CVMode: %d\n", CVmodeFlag);
                    break;
                }
            }
        }
    }
}

bool setup(BelaContext *context, void *userData)
{
    //Sleep for waiting boot BLOCKS
    std::cout<<"Begin sleep"<<std::endl;
    sleep(2);
    std::cout<<"End sleep"<<std::endl;
    
    //Digital pins setup
    pinMode(context, 0, P8_07, INPUT);//AudioModeA
    pinMode(context, 0, P8_09, INPUT);//AudioModeB
    pinMode(context, 0, P8_11, INPUT);//AudioModeC
    pinMode(context, 0, P8_15, INPUT);//AudioModeD
    pinMode(context, 0, P8_08, INPUT);//CVModeA
    pinMode(context, 0, P8_10, INPUT);//CVModeB
    pinMode(context, 0, P8_12, INPUT);//CVModeC
    pinMode(context, 0, P8_16, INPUT);//CVModeD
    pinMode(context, 0, P8_16, INPUT);//Audio/CV switching
    
    //MIDI
    midi.readFrom(gMidiPort0);
    midi.writeTo(gMidiPort0);
    midi.enableParser(true);
    midi.setParserCallback(midiMessageCallback, (void *)gMidiPort0);
    
    samplePlay_buffer = new StereoBuffer[NUMSAMPLEPLAYBUFFER];
    
    //Load Sample
    samplePlay_buffer[0].loadSampleFile("samplePlayA.wav");
    samplePlay_buffer[1].loadSampleFile("samplePlayB.wav");
    samplePlay_buffer[2].loadSampleFile("samplePlayC.wav");
    samplePlay_buffer[3].loadSampleFile("samplePlayD.wav");
    granular.loadFile("GranularSource.wav");
    
    outputGain.set(0.01f);
    return true;
}

void render(BelaContext *context, void *userData)
{
	//test用強制モード切替
	// midi_byte_t audioModeBytes[3] = {0xBF, (midi_byte_t)(1), 48};//Channel:16, CC Number:1, Value:48
 	// midi.writeOutput(audioModeBytes, 3);
 	// midi_byte_t cvModeBytes[3] = {0xBF, (midi_byte_t)(2), 80};//Channel:16, CC Number:2, Value:80
 	// midi.writeOutput(cvModeBytes, 3);
	
	
	
/*===========================================
Digital
=============================================*/
	bool audiopage = (bool)digitalRead(context, 0, P9_12);
	if(audiopage != isAudioPage) {
        midi_byte_t bp[3] = {0xBF, (midi_byte_t)(8), 0};//Channel:16, CC Number:8
        if(audiopage) {
        	bp[2] = 0;
        }
        else {
        	bp[2] = 127;
        }
        midi.writeOutput(bp, 3);
        isAudioPage = audiopage;
	}

    unsigned char audioFLG = AudioModeB;
    unsigned char cvFLG = CVModeD;

    //Audio
    if(digitalRead(context, 0, P8_07)) audioFLG = AudioModeA;
    if(digitalRead(context, 0, P8_09)) audioFLG = AudioModeB;
    if(digitalRead(context, 0, P8_11)) audioFLG = AudioModeC;
    if(digitalRead(context, 0, P8_15)) audioFLG = AudioModeD;
    if(AudiomodeFlag != audioFLG && audioFLG != 0) {
        midi_byte_t bytes[3] = {0xBF, (midi_byte_t)(1), 0};//Channel:16, CC Number:1, Value:0
        if(audioFLG == AudioModeA) {
            //Granular
            bytes[2] = 16;
        }
        else if(audioFLG == AudioModeB) {
            //Sample playback
            bytes[2] = 48;
        }
        else if(audioFLG == AudioModeC) {
            //Karplus strong
            bytes[2] = 80;
        }
        else if(audioFLG == AudioModeD) {
            //Logistic map
            bytes[2] = 112;
        }
        midi.writeOutput(bytes, 3);
        // midi_byte_t w[3] = {0xBF, (midi_byte_t)(8), 127};//BLOCKS画面切替用(Channel:16, CC Number:8, Value:127)
        // midi.writeOutput(w, 3);
        AudiomodeFlag = audioFLG;
    }
    
    //CV
    if(digitalRead(context, 0, P8_08)) cvFLG = CVModeA;
    if(digitalRead(context, 0, P8_10)) cvFLG = CVModeB;
    if(digitalRead(context, 0, P8_12)) cvFLG = CVModeC;
    if(digitalRead(context, 0, P8_16)) cvFLG = CVModeD;
    if(CVmodeFlag != cvFLG && cvFLG != 0) {
        midi_byte_t bytes[3] = {0xBF, (midi_byte_t)(2), 0};//Channel:16, CC Number:2, Value:0
        if(cvFLG == CVModeA) {
            //Morph looper
            bytes[2] = 16;
        }
        else if(cvFLG == CVModeB) {
            //Microtonal
            bytes[2] = 48;
        }
        else if(cvFLG == CVModeC) {
            //CVmode3
            bytes[2] = 80;
        }
        else if(cvFLG == CVModeD) {
            //Euclid sequence
            bytes[2] = 112;
        }
        midi.writeOutput(bytes, 3);
        // midi_byte_t w[3] = {0xBF, (midi_byte_t)(8), 127};//BLOCKS画面切替用(Channel:16, CC Number:8, Value:0)
        // midi.writeOutput(w, 3);
        CVmodeFlag = cvFLG;
    }
    
/*===========================================
Analogue
=============================================*/
//AnalogueIN
    const int numAnalogueFrames = context->analogFrames;
    float outGain = 0.0f;
    float paramKnobA = 0.0f;
    float paramKnobB = 0.0f;
    for(unsigned int n = 0; n < numAnalogueFrames; n++) {
        outGain += analogRead(context, n, 0);
        paramKnobA += analogRead(context, n, 1);
        paramKnobB += analogRead(context, n, 2);
    }
    outGain = outGain / (float)numAnalogueFrames;
    paramKnobA = paramKnobA / (float)numAnalogueFrames;//Parameter Knob A
    paramKnobB = paramKnobB / (float)numAnalogueFrames;//Parameter Knob B
    outputGain.set(outGain);
    
//AnalogueOUT
    switch(CVmodeFlag) {
        case CVModeA: {
            //Morph looper
            break;
        }
        case CVModeB: {
            //Microtonal
            for(unsigned int n = 0; n < numAnalogueFrames; n++) {
                for(unsigned ch = 0; ch < NUMCVOUT; ch++) {
                    analogWrite(context, n, ch, CVSmooth[ch].getNextValue());
                }
            }
            break;
        }
        case CVModeC: {
            break;
        }
        case CVModeD: {
            //Euclid sequence
            break;
        }
        default: {
            rt_printf("CVMode: %d\n", CVmodeFlag);
            break;
        }
    }
    
    
/*===========================================
Audio
=============================================*/
    const int numAudioFrames = context->audioFrames;
    switch(AudiomodeFlag) {
        case AudioModeA: {
            //Granular
            float gr[numAudioFrames];
            for(unsigned int i = 0; i < numAudioFrames; ++i) {
                gr[i] = 0.0f;
            }
            granular.nextBlock(gr, numAudioFrames);
            
            for(unsigned int i = 0; i < numAudioFrames; ++i) {
            	const float gain = outputGain.getNextValue();
                audioWrite(context, i, 0, gr[i] * gain);
                audioWrite(context, i, 1, gr[i] * gain);
            }
            break;
        }
        case AudioModeB: {
            //Sample playback
            float l[numAudioFrames];
            float r[numAudioFrames];
            for(unsigned int sample = 0; sample < numAudioFrames; ++sample) {
            	l[sample] = 0.0f;
            	r[sample] = 0.0f;
            }
            
            for(unsigned int i = 0; i < 4; ++i) {
            	if(samplePlay_isPlaying[i]) {
            		samplePlay_buffer[i].nextBlock(l, r, numAudioFrames);
            		if(samplePlay_buffer[i].isBufferEnd()) samplePlay_isPlaying[i] = false;
            	}
            }
            
            for(unsigned int sample = 0; sample < numAudioFrames; ++sample) {
            	const float gain = outputGain.getNextValue();
                audioWrite(context, sample, 0, l[sample] * gain);
                audioWrite(context, sample, 1, r[sample] * gain);
            }
            break;
        }
        case AudioModeC: {
            //Karplus strong
            break;
        }
        case AudioModeD: {
            //Logistic map
            break;
        }
        default: {
            rt_printf("AudioMode: %d\n", AudiomodeFlag);
            break;
        }
    }
}

void cleanup(BelaContext *context, void *userData)
{
	delete [] samplePlay_buffer;
}