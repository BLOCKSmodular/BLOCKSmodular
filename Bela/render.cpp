//ver0.6.5
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

static constexpr int NUMCVOUT = 8;
static constexpr int NUMSAMPLEPLAYBUFFER = 4;
static constexpr int NUMKARPLUSVOICE = 4;
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
bool isCVPage = false;
Midi midi;
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
HighResolutionControlChange microtone_Distance[4];
HighResolutionControlChange microtone_Pressure[4];
StereoBuffer* samplePlay_buffer;
bool samplePlay_isPlaying[4]{false, false, false, false};
KarplusStrong karplus[NUMKARPLUSVOICE];
HighResolutionControlChange kp_pitch[NUMKARPLUSVOICE];
HighResolutionControlChange kp_decay[NUMKARPLUSVOICE];
Smoothing CVSmooth[NUMCVOUT];


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
        //std::cout<<channel<<", "<<controlNum<<", "<<value<<std::endl;
        
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
                    		samplePlay_buffer[voiceIndex].setReadIter(0);
                    		samplePlay_isPlaying[voiceIndex] = true;
                    	}
                    }
                    break;
                }
                case AudioModeC: {
                    //Karplus strong
                    if(controlNum == 1 || controlNum == 2) {
                    	bool isUpeerByte{controlNum == 1};
                        kp_pitch[voiceIndex].set(value, isUpeerByte);
                        if(kp_pitch[voiceIndex].update()) {
                        		const float p = kp_pitch[voiceIndex].get() * 140.0f + 28.0f;
                        		karplus[voiceIndex].setFreq(p);
                    	}
                    }
                    
                    if(controlNum == 3 || controlNum == 4) {
                    	bool isUpeerByte{controlNum == 3};
                        kp_decay[voiceIndex].set(value, isUpeerByte);
                        if(kp_decay[voiceIndex].update()) karplus[voiceIndex].setDecay(kp_decay[voiceIndex].get());
                    }
                    	
                    if(controlNum == 5 && value == 127) {
                    	karplus[voiceIndex].trigger();
                    }
                    
                    break;
                }
                case AudioModeD: {
                    //Logistic map
                    if(voiceIndex == 0) {
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
                    else if(voiceIndex == 1) {
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
Digital
=============================================*/
	bool cvpage = (bool)digitalRead(context, 0, P9_12);
	if(cvpage != isCVPage) {
        midi_byte_t bp[3] = {0xBF, (midi_byte_t)(8), 0};//Channel:16, CC Number:8
        if(cvpage) {
        	bp[2] = 127;
        }
        else {
        	bp[2] = 0;
        }
        midi.writeOutput(bp, 3);
        isCVPage = cvpage;
	}

    unsigned char audioFLG = 0;
    unsigned char cvFLG = 0;

    //Audio
    if(digitalRead(context, 0, P8_07)) audioFLG = AudioModeA;
    if(digitalRead(context, 0, P8_09)) audioFLG = AudioModeB;
    if(digitalRead(context, 0, P8_11)) audioFLG = AudioModeC;
    if(digitalRead(context, 0, P8_15)) audioFLG = AudioModeD;
    if(AudiomodeFlag != audioFLG && audioFLG != 0) {
        midi_byte_t bytes[3] = {0xBF, (midi_byte_t)(1), 0};//Channel:16, CC Number:1
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
        AudiomodeFlag = audioFLG;
    }
    
    //CV
    if(digitalRead(context, 0, P8_08)) cvFLG = CVModeA;
    if(digitalRead(context, 0, P8_10)) cvFLG = CVModeB;
    if(digitalRead(context, 0, P8_12)) cvFLG = CVModeC;
    if(digitalRead(context, 0, P8_16)) cvFLG = CVModeD;
    if(CVmodeFlag != cvFLG && cvFLG != 0) {
        midi_byte_t bytes[3] = {0xBF, (midi_byte_t)(2), 0};//Channel:16, CC Number:2
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
        CVmodeFlag = cvFLG;
    }
    
/*===========================================
Analogue
=============================================*/
//AnalogueIN
    const int numAnalogueFrames = context->analogFrames;
    
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
            // rt_printf("CVMode: %d\n", CVmodeFlag);
            break;
        }
    }
    
    
/*===========================================
Audio
=============================================*/
    const int numAudioFrames = context->audioFrames;
    float gr[numAudioFrames];
    float l[numAudioFrames];
    float r[numAudioFrames];
    float kpbuf[numAudioFrames];
    float noiseBuf[numAudioFrames];
    for(unsigned int i = 0; i < numAudioFrames; ++i) {
    	gr[i] = 0.0f;
    	l[i] = 0.0f;
    	r[i] = 0.0f;
    	kpbuf[i] = 0.0f;
    	noiseBuf[i] = 0.0f;
    }
    
    //granular
   	granular.nextBlock(gr, numAudioFrames);
   	
   	//Sample playback
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
   	
   	//Karplus Strong
	for(unsigned int i = 0; i < NUMKARPLUSVOICE; ++i) {
		karplus[i].nextBlock(kpbuf, numAudioFrames);
  	}


  	//Logistic map  	
  	for(unsigned int sample = 0; sample < numAudioFrames; ++sample) {
        float v = 0.0f;
        v += logisticOsc.update();
        v += sineCircleOsc.update();
        noiseBuf[sample] = v;
  	}
     
     
    for(unsigned int i = 0; i < numAudioFrames; ++i) {
    	audioWrite(context, i, 0, gr[i] * 0.4f + l[i] + kpbuf[i] * 0.3f + noiseBuf[i] * 0.5f);
    	audioWrite(context, i, 1, gr[i] * 0.4f + r[i] + kpbuf[i] * 0.3f + noiseBuf[i] * 0.5f);
  	}   
    
    // switch(AudiomodeFlag) {
    //     case AudioModeA: {
    //         //Granular
    //         float gr[numAudioFrames];
    //         for(unsigned int i = 0; i < numAudioFrames; ++i) {
    //             gr[i] = 0.0f;
    //         }
    //         granular.nextBlock(gr, numAudioFrames);
            
    //         for(unsigned int i = 0; i < numAudioFrames; ++i) {
    //         	const float gain = 0.01f;
    //             audioWrite(context, i, 0, gr[i] * gain);
    //             audioWrite(context, i, 1, gr[i] * gain);
    //         }
    //         break;
    //     }
    //     case AudioModeB: {
    //         //Sample playback
    //         float l[numAudioFrames];
    //         float r[numAudioFrames];
    //         for(unsigned int sample = 0; sample < numAudioFrames; ++sample) {
    //         	l[sample] = 0.0f;
    //         	r[sample] = 0.0f;
    //         }
            
    //         for(unsigned int i = 0; i < 4; ++i) {
    //         	if(samplePlay_isPlaying[i]) {
    //         		samplePlay_buffer[i].nextBlock(l, r, numAudioFrames);
    //         		if(samplePlay_buffer[i].isBufferEnd()) samplePlay_isPlaying[i] = false;
            		
    //         	}
    //         }
            
    //         for(unsigned int sample = 0; sample < numAudioFrames; ++sample) {
    //         	const float gain = 0.1f;
    //             audioWrite(context, sample, 0, l[sample] * gain);
    //             audioWrite(context, sample, 1, r[sample] * gain);
    //         }
    //         break;
    //     }
    //     case AudioModeC: {
    //         //Karplus strong
    //         float b[numAudioFrames];
    //         for(unsigned int i = 0; i < numAudioFrames; ++i) {
    //             b[i] = 0.0f;
    //         }
            
    //         for(unsigned int i = 0; i < NUMKARPLUSVOICE; ++i) {
    //         	karplus[i].nextBlock(b, numAudioFrames);
    //         }
            
    //         for(unsigned int i = 0; i < numAudioFrames; ++i) {
    //         	const float v = b[i] * 0.1f;
    //             audioWrite(context, i, 0, v);
    //             audioWrite(context, i, 1, v);
    //         }
    //         break;
    //     }
    //     case AudioModeD: {
    //         //Logistic map
    //         for(unsigned int sample = 0; sample < numAudioFrames; ++sample) {
    //         	float v = 0.0f;
    //         	v += logisticOsc.update();
    //         	v += sineCircleOsc.update();
    //         	v = v * 0.1f;
    //             audioWrite(context, sample, 0, v);
    //             audioWrite(context, sample, 1, v);
    //         }
    //         break;
    //     }
    //     default: {
    //         // rt_printf("AudioMode: %d\n", AudiomodeFlag);
    //         break;
    //     }
    // }
}

void cleanup(BelaContext *context, void *userData)
{
	delete [] samplePlay_buffer;
}
