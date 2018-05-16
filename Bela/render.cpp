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
#include <KarplusStrong.h>

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
static constexpr int NumVoice_PhysicalDrum = 4;
static constexpr int NumVoice_Microtone = 4;
static constexpr int NumVoice_ChaoticNoise = 4;

enum class ModeList{
    init = 0,
    Microtone
    Euclid,
    ChaoticNoise,
    PhysicalDrum,
    Granular
};

Midi midi;
ModeList mode;
const char *gMidiPort0 = "hw:1,0,0";

LogisticMap logisticOsc[NumVoice_ChaoticNoise];
HighResolutionControlChange	logistic_Alpha[NumVoice_ChaoticNoise];
HighResolutionControlChange logistic_Gain[NumVoice_ChaoticNoise];
GranularSynth granular;
HighResolutionControlChange granular_Position[2];
HighResolutionControlChange granular_GrainSize[2];
HighResolutionControlChange granular_Density[2];
HighResolutionControlChange microtone_Distance[NumVoice_Microtone];
HighResolutionControlChange microtone_Pressure[NumVoice_Microtone];
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
                break;
            }
                
            case ModeList::ChaoticNoise: {
                if(channel >= NumVoice_ChaoticNoise) {
                    std::cout<<"MIDI: Invalid voice number"<<std::endl;
                    break;
                }
                if(controlNum == 1 || controlNum == 2) {
                    bool isUpeerByte{controlNum == 1};
                    logistic_Alpha.set(value, isUpeerByte);
                    if(logistic_Alpha.update()) logisticOsc[channel].setAlpha(logistic_Alpha.get() * 0.5f + 3.490f);
                }
                
                if(controlNum == 3 || controlNum == 4) {
                    bool isUpeerByte{controlNum == 3};
                    logistic_Gain.set(value, isUpeerByte);
                    if(logistic_Gain.update()) logisticOsc[channel].setGain(logistic_Gain.get());
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
                    granular_Position[channel].set(value, isUpeerByte);
                    if(granular_Position[channel].update()) granular.setBufferPosition(granular_Position[channel].get(), channel);
                }
                
                if(controlNum == 3 || controlNum == 4) {
                    bool isUpeerByte{controlNum == 3};
                    granular_GrainSize[channel].set(value, isUpeerByte);
                    if(granular_GrainSize[channel].update()) granular.setGrainSize(granular_GrainSize[channel].get(), channel);
                }
                
                if(controlNum == 5 || controlNum == 6) {
                    bool isUpeerByte{controlNum == 5};
                    granular_Density[channel].set(value, isUpeerByte);
                    if(granular_Density[channel].update()) {
                        float v = granular_Density[channel].get() * 2.0f;
                        granular.setWindowShape(v * 4.0f, channel);
                        granular.setDensity((v * v * v), channel);
                    }
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
    int* modeFlag = nullptr;
    if(digitalRead(context, 0, pin_microtone)) modeFlag = static_cast<int*>ModeList::Microtone;
    if(digitalRead(context, 0, pin_euclid)) modeFlag = static_cast<int*>ModeList::Euclid;
    if(digitalRead(context, 0, pin_chaoticNoise)) modeFlag = static_cast<int*>ModeList::ChaoticNoise;
    if(digitalRead(context, 0, pin_physicalDrum)) modeFlag = static_cast<int*>ModeList::PhysicalDrum;
    if(digitalRead(context, 0, pin_granular)) modeFlag = static_cast<int*>ModeList::Granular;
    if(mode != static_cast<ModeList>(modeFlag) && modeFlag != nullptr) {
        midi_byte_t bytes[3] = {0xBF, (midi_byte_t)(1), 0};//Channel:16, CC Number:1
        bytes[2] = modeFlag;
        midi.writeOutput(bytes, 3);
        mode = static_cast<ModeList>(modeFlag);
    }
    
    /*===========================================
     CV Output
     =============================================*/
    const int numAnalogueFrames = context->analogFrames;
    switch(mode) {
        case ModeList:: Microtone: {
            for(unsigned int n = 0; n < numAnalogueFrames; n++) {
                for(unsigned ch = 0; ch < NumOutput_CV; ch++) {
                    analogWrite(context, n, ch, CVSmooth[ch].getNextValue());
                }
            }
            break;
        }
        default: {
            //rt_printf("CVOut Switch: default...\n");
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
    for(unsigned int i = 0; i < NumVoice_PhysicalDrum; ++i) {//Karplus Strong
        physicalDrum[i].nextBlock(kpBuf, numAudioFrames);
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
}
