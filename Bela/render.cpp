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
static constexpr unsigned char CVModeA =   0b00000001;
static constexpr unsigned char CVModeB =   0b00000010;
static constexpr unsigned char CVModeOFF = 0b00000100;
static constexpr unsigned char CVModeC =   0b00001000;
static constexpr unsigned char CVModeD =   0b00010000;
static constexpr unsigned char AudioModeA =   0b00000001;
static constexpr unsigned char AudioModeB =   0b00000010;
static constexpr unsigned char AudioModeOFF = 0b00000100;
static constexpr unsigned char AudioModeC =   0b00001000;
static constexpr unsigned char AudioModeD =   0b00010000;
unsigned char CVmodeFlag;
unsigned char AudiomodeFlag;
Midi midi;
const char *gMidiPort0 = "hw:1,0,0";
MonoBuffer monoBuffer(88200, true, false);
StereoBuffer stereoBuffer(88200, true, false);
GranularSynth granular;
HighResolutionControlChange gr_Position[2];
HighResolutionControlChange gr_GrainSize[2];
HighResolutionControlChange gr_WindowShape[2];
HighResolutionControlChange microtone_Distance[4];
HighResolutionControlChange microtone_Pressure[4];
Smoothing CVSmooth[NUMCVOUT];

void midiMessageCallback(MidiChannelMessage message, void *arg)
{
	const int channel = message.getChannel();//MIDIChannel 1~16
	
	//Note On
	if(message.getType() == kmmNoteOn){
	}

	//Control change
    if(message.getType() == kmmControlChange)
    {
        const int controlNum = message.getDataByte(0);
        const int value = message.getDataByte(1);
        
        if(channel == 16) {
            //General messeges
        }
        else if(channel < 9) {
            //Audio
            const int voiceIndex = channel - 1;
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
                    break;
                }
                case AudioModeOFF: {
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
            const int voiceIndex = channel - 9;
            //CV
            switch(CVmodeFlag) {
                case CVModeA: {
                    //Morph looper
                    break;
                }
                case CVModeB: {
                    //Microtonal
                    if(controlNum == 1 || controlNum == 2) {
                        bool isUpeerByte{controlNum == 1};
                        microtone_Distance[voiceIndex].set(value, isUpeerByte);
                        if(microtone_Distance[voiceIndex].update()) CVSmooth[voiceIndex].set(microtone_Distance[voiceIndex].get());
                    }
                    
                    if(controlNum == 3 || controlNum == 4) {
                        bool isUpeerByte{controlNum == 3};
                        microtone_Pressure[voiceIndex].set(value, isUpeerByte);
                        if(microtone_Pressure[voiceIndex].update()) CVSmooth[voiceIndex].set(microtone_Pressure[voiceIndex].get());
                    }
                    break;
                }
                case CVModeOFF: {
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
    
    CVmodeFlag = CVModeA;
    AudiomodeFlag = AudioModeA;
    
    //Digital pins setup
    pinMode(context, 0, P8_07, INPUT);//AudioModeA
    pinMode(context, 0, P8_08, INPUT);//AudioModeB
    pinMode(context, 0, P8_09, INPUT);//AudioModeOFF
    pinMode(context, 0, P8_10, INPUT);//AudioModeC
    pinMode(context, 0, P8_11, INPUT);//AudioModeD
    pinMode(context, 0, P8_18, INPUT);//CVModeA
    pinMode(context, 0, P8_27, INPUT);//CVModeB
    pinMode(context, 0, P8_28, INPUT);//CVModeOFF
    pinMode(context, 0, P8_29, INPUT);//CVModeC
    pinMode(context, 0, P8_30, INPUT);//CVModeD
    
    //MIDI
    midi.readFrom(gMidiPort0);
    midi.writeTo(gMidiPort0);
    midi.enableParser(true);
    midi.setParserCallback(midiMessageCallback, (void *)gMidiPort0);
    
    midi_byte_t bytes[3] = {176, (midi_byte_t)(97), 127};
    midi.writeOutput(bytes, 3);
    
    //Load Sample
    monoBuffer.loadSampleFile("VibeOneshot.wav");
    stereoBuffer.loadSampleFile("BellRoll.wav");
    granular.loadFile("GranularSource.wav");
    
    return true;
}

void render(BelaContext *context, void *userData)
{
    //-----------------------------------------------------------
    //Digital
    unsigned char cvFLG = 0;
    unsigned char audioFLG = 0;
    //TODOチャタリング除去
    //CV
    if(digitalRead(context, 0, P8_07)) cvFLG = CVModeA;
    if(digitalRead(context, 0, P8_08)) cvFLG = CVModeB;
    if(digitalRead(context, 0, P8_09)) cvFLG = CVModeOFF;//P8_09はOFFスイッチ
    if(digitalRead(context, 0, P8_10)) cvFLG = CVModeC;
    if(digitalRead(context, 0, P8_11)) cvFLG = CVModeD;
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
        else if(cvFLG == CVModeOFF) {
            //CVmode OFF
            bytes[2] = 0;
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
        midi_byte_t w[3] = {0xBF, (midi_byte_t)(8), 127};//BLOCKS画面切替用(Channel:16, CC Number:8, Value:0)
        midi.writeOutput(w, 3);
        CVmodeFlag = cvFLG;
    }
    //Audio
    if(digitalRead(context, 0, P8_18)) audioFLG = AudioModeA;
    if(digitalRead(context, 0, P8_27)) audioFLG = AudioModeB;
    if(digitalRead(context, 0, P8_28)) audioFLG = AudioModeOFF;//P8_28はOFFスイッチ
    if(digitalRead(context, 0, P8_29)) audioFLG = AudioModeC;
    if(digitalRead(context, 0, P8_30)) audioFLG = AudioModeD;
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
        else if(audioFLG == AudioModeOFF) {
            //Audiomode OFF
            bytes[2] = 0;
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
        midi_byte_t w[3] = {0xBF, (midi_byte_t)(8), 127};//BLOCKS画面切替用(Channel:16, CC Number:8, Value:127)
        midi.writeOutput(w, 3);
        AudiomodeFlag = audioFLG;
    }
    
    //-----------------------------------------------------------
    //Analogue
    const int numAnalogueFrames = context->analogFrames;
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
        case CVModeOFF: {
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
    
    
    //-----------------------------------------------------------
    //Audio
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
                audioWrite(context, i, 0, gr[i] * 0.1f);
                audioWrite(context, i, 1, gr[i] * 0.1f);
            }
            break;
        }
        case AudioModeB: {
            //Sample playback
            float mono = 0.0f;
            for(unsigned int i = 0; i < numAudioFrames; ++i) {
                monoBuffer.readNext(mono);
                audioWrite(context, i, 0, mono);
                audioWrite(context, i, 1, mono);
            }
            
            float stereo[2] = {0.0f, 0.0f};
            for(unsigned int i = 0; i < numAudioFrames; ++i) {
                stereoBuffer.readNext(stereo[0], stereo[1]);
                audioWrite(context, i, 0, stereo[0]);
                audioWrite(context, i, 1, stereo[1]);
            }
            break;
        }
        case AudioModeOFF: {
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
}
