/*
 BLOCKSmodular ver0.8.0 Beta1
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
#include <SampleBuffer.h>
#include <Util.h>
#include <GranularSynth.h>
#include <LogisticMap.h>
#include <KarplusStrong.h>
#include <EuclideanRhythm.h>

#define pin_microtone P8_27
#define pin_euclid P9_12
#define pin_chaoticNoise P9_14
#define pin_physicalDrum P9_16
#define pin_granular P8_29
#define pin_gate1 P8_08
#define pin_gate2 P8_10
#define pin_gate3 P8_12
#define pin_gate4 P8_16
static constexpr int NumOutput_CV = 8;
static constexpr int NumOutput_Gate = 4;
static constexpr int NumVoice_Microtone = 4;
static constexpr int NumVoice_Euclid = 4;
static constexpr int NumVoice_ChaoticNoise = 4;
static constexpr int NumVoice_PhysicalDrum = 4;

enum class ModeList{
    init = 0,
    Microtone,
    Euclid,
    ChaoticNoise,
    PhysicalDrum,
    Granular
};

Midi midi;
ModeList mode;
const char *gMidiPort0 = "hw:1,0,0";
HighResolutionControlChange microtone_Distance[NumVoice_Microtone];
HighResolutionControlChange microtone_Pressure[NumVoice_Microtone];
EuclideanRhythm euclid[NumVoice_Euclid];
HighResolutionControlChange euclid_Tempo[NumVoice_Euclid];
LogisticMap logisticOsc[NumVoice_ChaoticNoise];
HighResolutionControlChange	logistic_Alpha[NumVoice_ChaoticNoise];
HighResolutionControlChange logistic_Gain[NumVoice_ChaoticNoise];
GranularSynth granular;
HighResolutionControlChange granular_Onset[2];
HighResolutionControlChange granular_GrainSize[2];
HighResolutionControlChange granular_Overlap[2];
KarplusStrong physicalDrum[NumVoice_PhysicalDrum];
HighResolutionControlChange physicalDrum_Pitch[NumVoice_PhysicalDrum];
HighResolutionControlChange physicalDrum_Decay[NumVoice_PhysicalDrum];
Smoothing CVSmooth[NumOutput_CV];

void midiMessageCallback(MidiChannelMessage message, void *arg)
{
    const int channel = message.getChannel();//MIDIChannel(0~15)
    if(message.getType() == kmmControlChange)//MIDI CC
    {
        const int controlNum = message.getDataByte(0);
        const int value = message.getDataByte(1);
        //std::cout<<channel<<", "<<controlNum<<", "<<value<<std::endl;
        
        switch(mode) {
            case ModeList::Microtone: {
                if(channel >= NumVoice_Microtone) {
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
                if(channel >= NumVoice_Euclid) {
                    std::cout<<"MIDI: Invalid voice number"<<std::endl;
                    break;
                }
                
                if (controlNum == 1) euclid[channel].setnumSteps((int)(value * 0.5));//0~63ステップ
                if (controlNum == 2) euclid[channel].setBeatRatio((float)value / 127.0f);//0.0f~1.0f
                if (controlNum == 3 || controlNum == 4) {
                    bool isUpperByte{controlNum == 3};
                    euclid_Tempo[channel].set(value, isUpperByte);
                    if(euclid_Tempo[channel].update()) euclid[channel].setBPM(euclid_Tempo[channel].get() * 100.0f + 60.0f);//BPM60~160
                }
                
                if(controlNum == 1 || controlNum == 2) euclid[channel].generateRhythm();
                break;
            }
                
            case ModeList::ChaoticNoise: {
                if(channel >= NumVoice_ChaoticNoise) {
                    std::cout<<"MIDI: Invalid voice number"<<std::endl;
                    break;
                }
                if(controlNum == 1 || controlNum == 2) {
                    bool isUpperByte{controlNum == 1};
                    logistic_Alpha[channel].set(value, isUpperByte);
                    if(logistic_Alpha[channel].update()) logisticOsc[channel].setAlpha(logistic_Alpha[channel].get() * 0.5f + 3.490f);
                }
                
                if(controlNum == 3 || controlNum == 4) {
                    bool isUpeerByte{controlNum == 3};
                    logistic_Gain[channel].set(value, isUpeerByte);
                    if(logistic_Gain[channel].update()) logisticOsc[channel].setGain(logistic_Gain[channel].get());
                }
                
                break;
            }
                
            case ModeList::PhysicalDrum: {
                if(channel >= NumVoice_PhysicalDrum) {
                    break;
                }
                
                if(controlNum == 1 || controlNum == 2) {
                    bool isUpeerByte{controlNum == 1};
                    physicalDrum_Pitch[channel].set(value, isUpeerByte);
                    if(physicalDrum_Pitch[channel].update()) {
                        const float p = physicalDrum_Pitch[channel].get() * 40.0f + 40.0f;//40Hz~80Hz
                        physicalDrum[channel].setFreq(p);
                    }
                }
                
                if(controlNum == 3 || controlNum == 4) {
                    bool isUpeerByte{controlNum == 3};
                    physicalDrum_Decay[channel].set(value, isUpeerByte);
                    if(physicalDrum_Decay[channel].update()) physicalDrum[channel].setDecay(physicalDrum_Decay[channel].get());
                }
                
                if(controlNum == 5 && value == 127) {
                    physicalDrum[channel].trigger();
                }
                
                break;
            }
                
                
            case ModeList::Granular: {
                if(channel >= 2) {
                    break;
                }
                
                if(controlNum == 1 || controlNum == 2) {
                    bool isUpeerByte{controlNum == 1};
                    granular_Onset[channel].set(value, isUpeerByte);
                    if(granular_Onset[channel].update()) granular.setBufferOnset(granular_Onset[channel].get(), channel);
                }
                
                if(controlNum == 3 || controlNum == 4) {
                    bool isUpeerByte{controlNum == 3};
                    granular_GrainSize[channel].set(value, isUpeerByte);
                    if(granular_GrainSize[channel].update()) granular.setGrainSize(granular_GrainSize[channel].get(), channel);
                }
                
                if(controlNum == 5 || controlNum == 6) {
                    bool isUpeerByte{controlNum == 5};
                    granular_Overlap[channel].set(value, isUpeerByte);
                    if(granular_Overlap[channel].update()) granular.setOverlap(granular_Overlap[channel].get(), channel);
                }
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
    mode = ModeList::init;
    for(int i = 0; i < NumOutput_Gate; ++i) {
        euclid[i].init(context->digitalSampleRate);
    }
    
    //Mode change pin setup
    pinMode(context, 0, pin_microtone, INPUT);//Microtone
    pinMode(context, 0, pin_euclid, INPUT);//Euclid
    pinMode(context, 0, pin_chaoticNoise, INPUT);//ChaoticNoise
    pinMode(context, 0, pin_physicalDrum, INPUT);//PhysicalDrum
    pinMode(context, 0, pin_granular, INPUT);//Granular
    
    //Gate output setup
    pinMode(context, 0, pin_gate1, OUTPUT);//Gate1
    pinMode(context, 0, pin_gate2, OUTPUT);//Gate2
    pinMode(context, 0, pin_gate3, OUTPUT);//Gate3
    pinMode(context, 0, pin_gate4, OUTPUT);//Gate4
    
    //MIDI
    midi.readFrom(gMidiPort0);
    midi.writeTo(gMidiPort0);
    midi.enableParser(true);
    midi.setParserCallback(midiMessageCallback, (void *)gMidiPort0);
    
    //Load Sample file
    granular.loadFile("GranularSource.wav");
    
    return true;
}

void render(BelaContext *context, void *userData)
{
    /*===========================================
     Mode change
     =============================================*/
    // int* modeFlag = nullptr;
    ModeList modeFlag = ModeList::init;
    if(digitalRead(context, 0, pin_microtone)) modeFlag = ModeList::Microtone;
    if(digitalRead(context, 0, pin_euclid)) modeFlag = ModeList::Euclid;
    if(digitalRead(context, 0, pin_chaoticNoise)) modeFlag = ModeList::ChaoticNoise;
    if(digitalRead(context, 0, pin_physicalDrum)) modeFlag = ModeList::PhysicalDrum;
    if(digitalRead(context, 0, pin_granular)) modeFlag = ModeList::Granular;
    if(mode != modeFlag && modeFlag != ModeList::init) {
        midi_byte_t bytes[3] = {0xBF, (midi_byte_t)(1), 0};//Channel:16, CC Number:1
        bytes[2] = static_cast<int>(modeFlag);
        midi.writeOutput(bytes, 3);
        mode = modeFlag;
    }
    
    /*===========================================
     Gate Output
     =============================================*/
    if(mode == ModeList::Euclid) {
        for(int sample = 0; sample < context->digitalFrames; ++sample) {
            for(int channel = 0; channel < NumOutput_Gate; ++channel) {
                digitalWriteOnce(context, sample, channel, euclid[channel].update());
            }
        }
    }
    
    /*===========================================
     CV Output
     =============================================*/
    if(mode == ModeList::Microtone) {
        for(int sample = 0; sample < context->analogFrames; ++sample) {
            for(int channel = 0; channel < NumOutput_CV; ++channel) {
                analogWrite(context, sample, channel, CVSmooth[channel].getNextValue());
            }
        }
    }
    
    /*===========================================
     Audio
     =============================================*/
    const int NumAudioFrames = context->audioFrames;
    float buf_ChaoticNoise[NumAudioFrames];
    float buf_PhysicalDrum[NumAudioFrames];
    float buf_Granular[NumAudioFrames];
    for(int sample = 0; sample < NumAudioFrames; ++sample) {//TODO 配列の0初期化の高速化
        buf_ChaoticNoise[sample] = 0.0f;
        buf_PhysicalDrum[sample] = 0.0f;
        buf_Granular[sample] = 0.0f;
    }
    
    for(int sample = 0; sample < NumAudioFrames; ++sample) {//Chaotic Noise
    	for(int channel = 0; channel < NumVoice_ChaoticNoise; ++channel) {
    		buf_ChaoticNoise[sample] += logisticOsc[channel].update();	
    	}
    }
    for(int i = 0; i < NumVoice_PhysicalDrum; ++i) {//Physical Drum
        physicalDrum[i].nextBlock(buf_PhysicalDrum, NumAudioFrames);
    }
    granular.nextBlock(buf_Granular, NumAudioFrames);//Granular
    for(int i = 0; i < NumAudioFrames; ++i) {//Mixer
        audioWrite(context, i, 0, buf_Granular[i] * 0.4f + buf_PhysicalDrum[i] * 0.3f + buf_ChaoticNoise[i] * 0.5f);
        audioWrite(context, i, 1, buf_Granular[i] * 0.4f + buf_PhysicalDrum[i] * 0.3f + buf_ChaoticNoise[i] * 0.5f);
    }
}

void cleanup(BelaContext *context, void *userData)
{
}
