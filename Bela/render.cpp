/*
 BLOCKSmodular ver0.7.0
 render.cpp for BLOCKSmodular
 Created by Akiyuki Okayasu
 License: GPLv3
 */
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
#include <LogisticMap.h>
#include <SineCircleMap.h>
#include <KarplusStrong.h>

enum class ModeList{
    init = 0,
    Granular,
    SamplePlay,
    Karplus,
    Logistic,
    SineCircle,
    MorphLooper,
    Microtonal,
    Euclid
};

static constexpr int NUMCVOUT = 8;
static constexpr int NUMSAMPLEPLAYBUFFER = 4;
static constexpr int NUMKARPLUSVOICE = 4;
static constexpr int NUMMICROTONALVOICE = 4;
Midi midi;
ModeList mode;
const char *gMidiPort0 = "hw:1,0,0";

LogisticMap logisticOsc;
HighResolutionControlChange	lgst_alpha;
HighResolutionControlChange lgst_gain;
SineCircleMap sineCircleOsc;
HighResolutionControlChange sineCircle_k;
HighResolutionControlChange sineCircle_gain;
GranularSynth granular;
HighResolutionControlChange gr_Position[2];
HighResolutionControlChange gr_GrainSize[2];
HighResolutionControlChange gr_WindowShape[2];
HighResolutionControlChange microtone_Distance[NUMMICROTONALVOICE];
HighResolutionControlChange microtone_Pressure[NUMMICROTONALVOICE];
StereoBuffer* samplePlay_buffer;
bool samplePlay_isPlaying[NUMSAMPLEPLAYBUFFER]{false, false, false, false};
KarplusStrong karplus[NUMKARPLUSVOICE];
HighResolutionControlChange kp_pitch[NUMKARPLUSVOICE];
HighResolutionControlChange kp_decay[NUMKARPLUSVOICE];
Smoothing CVSmooth[NUMCVOUT];

void midiMessageCallback(MidiChannelMessage message, void *arg)
{
    const int channel = message.getChannel();//MIDIChannel(0~15)
    if(message.getType() == kmmControlChange)//MIDI CC
    {
        const int controlNum = message.getDataByte(0);
        const int value = message.getDataByte(1);
        //std::cout<<channel<<", "<<controlNum<<", "<<value<<std::endl;
        
        switch(mode) {
            case ModeList::init: {
                std::cout<<"mode init..."<<std::endl;
                break;
            }
            case ModeList::Granular: {
                if(channel >= 2)
                {
                    break;
                }
                
                if(controlNum == 1 || controlNum == 2) {
                    bool isUpeerByte{controlNum == 1};
                    gr_Position[channel].set(value, isUpeerByte);
                    if(gr_Position[channel].update()) granular.setBufferPosition(gr_Position[channel].get(), channel);
                }
                
                if(controlNum == 3 || controlNum == 4) {
                    bool isUpeerByte{controlNum == 3};
                    gr_GrainSize[channel].set(value, isUpeerByte);
                    if(gr_GrainSize[channel].update()) granular.setGrainSize(gr_GrainSize[channel].get(), channel);
                }
                
                if(controlNum == 5 || controlNum == 6) {
                    bool isUpeerByte{controlNum == 5};
                    gr_WindowShape[channel].set(value, isUpeerByte);
                    if(gr_WindowShape[channel].update()) {
                        float v = gr_WindowShape[channel].get() * 2.0f;
                        granular.setWindowShape(v * 4.0f, channel);
                        granular.setDensity((v * v * v), channel);
                    }
                }
                
                break;
            }
            case ModeList::SamplePlay: {
                if(channel >= NUMSAMPLEPLAYBUFFER)
                {
                    break;
                }
                
                if(controlNum == 1) {
                    if(value == 127) {
                        samplePlay_buffer[channel].setReadIter(0);
                        samplePlay_isPlaying[channel] = true;
                    }
                }
                
                break;
            }
            case ModeList::Karplus: {
                if(channel >= NUMKARPLUSVOICE)
                {
                    break;
                }
                
                if(controlNum == 1 || controlNum == 2) {
                    bool isUpeerByte{controlNum == 1};
                    kp_pitch[channel].set(value, isUpeerByte);
                    if(kp_pitch[channel].update()) {
                        const float p = kp_pitch[channel].get() * 40.0f + 40.0f;//40Hz~80Hz
                        karplus[channel].setFreq(p);
                    }
                }
                
                if(controlNum == 3 || controlNum == 4) {
                    bool isUpeerByte{controlNum == 3};
                    kp_decay[channel].set(value, isUpeerByte);
                    if(kp_decay[channel].update()) karplus[channel].setDecay(kp_decay[channel].get());
                }
                
                if(controlNum == 5 && value == 127) {
                    karplus[channel].trigger();
                }
                
                break;
            }
            case ModeList::Logistic: {
                if(channel == 0) {
                    if(controlNum == 1 || controlNum == 2) {
                        bool isUpeerByte{controlNum == 1};
                        lgst_alpha.set(value, isUpeerByte);
                        if(lgst_alpha.update()) logisticOsc.setAlpha(lgst_alpha.get() * 0.5f + 3.490f);
                    }
                    
                    if(controlNum == 3 || controlNum == 4) {
                        bool isUpeerByte{controlNum == 3};
                        lgst_gain.set(value, isUpeerByte);
                        if(lgst_gain.update()) logisticOsc.setGain(lgst_gain.get());
                    }
                }
                else if(channel == 1) {
                    if(controlNum == 1 || controlNum == 2) {
                        bool isUpeerByte{controlNum == 1};
                        sineCircle_k.set(value, isUpeerByte);
                        if(sineCircle_k.update()) sineCircleOsc.setK(sineCircle_k.get() * 0.2f + 1.0f);
                    }
                    
                    if(controlNum == 3 || controlNum == 4) {
                        bool isUpeerByte{controlNum == 3};
                        sineCircle_gain.set(value, isUpeerByte);
                        if(sineCircle_gain.update()) sineCircleOsc.setGain(sineCircle_gain.get());
                    }
                }
                break;
            }
            case ModeList::SineCircle: {
                //TODO SineCircleMapモードの実装
                break;
            }
            case ModeList::MorphLooper: {
                break;
            }
            case ModeList::Microtonal: {
                if(channel >= NUMMICROTONALVOICE) {
                    std::cout<<"MIDI: Invalid voice number"<<std::endl;
                    break;
                }
                
                if(controlNum == 1 || controlNum == 2) {
                    bool isUpeerByte{controlNum == 1};
                    microtone_Distance[channel].set(value, isUpeerByte);
                    if(microtone_Distance[channel].update()) CVSmooth[channel * 2].set(microtone_Distance[channel].get());
                }
                
                if(controlNum == 3 || controlNum == 4) {
                    bool isUpeerByte{controlNum == 3};
                    microtone_Pressure[channel].set(value, isUpeerByte);
                    if(microtone_Pressure[channel].update()) CVSmooth[channel * 2 + 1].set(microtone_Pressure[channel].get());
                }
                break;
            }
            case ModeList::Euclid: {
                break;
            }
            default: {
                break;
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
    pinMode(context, 0, P8_07, INPUT);//Granular
    pinMode(context, 0, P8_08, INPUT);//Sample playback
    pinMode(context, 0, P8_09, INPUT);//Karplus strong
    pinMode(context, 0, P8_10, INPUT);//Logistic
    pinMode(context, 0, P8_11, INPUT);//Sine circle
    pinMode(context, 0, P8_12, INPUT);//Morph looper
    pinMode(context, 0, P8_15, INPUT);//Microtonal
    pinMode(context, 0, P8_16, INPUT);//Euclid
    pinMode(context, 0, P8_18, INPUT);//blank...
    pinMode(context, 0, P8_27, INPUT);//blank...
    
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
    
    return true;
}

void render(BelaContext *context, void *userData)
{
    //---------------------------------------
    //test用強制モード切替
    // AudiomodeFlag = AudioModeB;
    // midi_byte_t audioModeBytes[3] = {0xBF, (midi_byte_t)(1), 48};//Channel:16, CC Number:1
    //	midi.writeOutput(audioModeBytes, 3);
    //	CVmodeFlag = CVModeB;
    //	midi_byte_t cvModeBytes[3] = {0xBF, (midi_byte_t)(2), 48};//Channel:16, CC Number:2
    //	midi.writeOutput(cvModeBytes, 3);
    //----------------------------------------
    
    /*===========================================
     Digital IN: Mode change
     =============================================*/
    int modeFlag = 0;
    if(digitalRead(context, 0, P8_07)) modeFlag = 1;//Granular
    if(digitalRead(context, 0, P8_08)) modeFlag = 2;//Sample playback
    if(digitalRead(context, 0, P8_09)) modeFlag = 3;//Karplus strong
    if(digitalRead(context, 0, P8_10)) modeFlag = 4;//Logistic
    if(digitalRead(context, 0, P8_11)) modeFlag = 5;//Sine circle
    if(digitalRead(context, 0, P8_12)) modeFlag = 6;//Morph looper
    if(digitalRead(context, 0, P8_15)) modeFlag = 7;//Microtonal
    if(digitalRead(context, 0, P8_16)) modeFlag = 8;//Euclid
    if(digitalRead(context, 0, P8_18)) modeFlag = 9;
    if(digitalRead(context, 0, P8_27)) modeFlag = 10;
    if(mode != static_cast<ModeList>(modeFlag) && modeFlag != 0) {
        midi_byte_t bytes[3] = {0xBF, (midi_byte_t)(1), 0};//Channel:16, CC Number:1
        bytes[2] = modeFlag;
        midi.writeOutput(bytes, 3);
        mode = static_cast<ModeList>(modeFlag);
    }
    
    /*===========================================
     Analogue OUT: CV/Gate
     =============================================*/
    const int numAnalogueFrames = context->analogFrames;
    switch(mode) {
        case ModeList::MorphLooper: {
            break;
        }
        case ModeList:: Microtonal: {
            for(unsigned int n = 0; n < numAnalogueFrames; n++) {
                for(unsigned ch = 0; ch < NUMCVOUT; ch++) {
                    analogWrite(context, n, ch, CVSmooth[ch].getNextValue());
                }
            }
            break;
        }
        case ModeList:: Euclid: {
            break;
        }
        default: {
            rt_printf("CVOut Switch: default...\n");
            break;
        }
    }
    
    /*===========================================
     Audio
     =============================================*/
    const int numAudioFrames = context->audioFrames;
    float grBuf[numAudioFrames];
    float sampLeftBuf[numAudioFrames];
    float sampRightBuf[numAudioFrames];
    float kpBuf[numAudioFrames];
    float noiseBuf[numAudioFrames];
    for(unsigned int i = 0; i < numAudioFrames; ++i) {
        grBuf[i] = 0.0f;
        sampLeftBuf[i] = 0.0f;
        sampRightBuf[i] = 0.0f;
        kpBuf[i] = 0.0f;
        noiseBuf[i] = 0.0f;
    }
    
    granular.nextBlock(grBuf, numAudioFrames);//granular
    for(unsigned int i = 0; i < 4; ++i) {//Sample playback
        if(samplePlay_isPlaying[i]) {
            samplePlay_buffer[i].nextBlock(sampLeftBuf, sampRightBuf, numAudioFrames);
            if(samplePlay_buffer[i].isBufferEnd()) samplePlay_isPlaying[i] = false;
        }
    }
    for(unsigned int i = 0; i < NUMKARPLUSVOICE; ++i) {//Karplus Strong
        karplus[i].nextBlock(kpBuf, numAudioFrames);
    }
    for(unsigned int sample = 0; sample < numAudioFrames; ++sample) {//Logistic map
        noiseBuf[sample] += logisticOsc.update();
        noiseBuf[sample] += sineCircleOsc.update();
    }
    for(unsigned int i = 0; i < numAudioFrames; ++i) {//mixer
        audioWrite(context, i, 0, grBuf[i] * 0.4f + sampLeftBuf[i] + kpBuf[i] * 0.3f + noiseBuf[i] * 0.5f);
        audioWrite(context, i, 1, grBuf[i] * 0.4f + sampRightBuf[i] + kpBuf[i] * 0.3f + noiseBuf[i] * 0.5f);
    }
}

void cleanup(BelaContext *context, void *userData)
{
    delete [] samplePlay_buffer;
}
