/*
 GranularSynth.h for BLOCKSmodular
 Created by Akiyuki Okayasu
 License: GPLv3
 */
#ifndef GranularSynth_H_
#define GranularSynth_H_

#include <math_neon.h>//Fast math function
#include <cmath>
#include <memory>
#include <array>
#include <vector>
#include <random>
#include <string>
#include <atomic>
#include "SampleBuffer.h"
#include "Util.h"

class GranularSynth {
public:
    GranularSynth()
    {
        buffer = std::make_unique<MonoBuffer>(44100, false, false);
        for(int i = 0; i < NumVoice; ++i) {
        	bufferOnset[i] = 0;
        	grainSize[i] = MaxGrainSizeInSamples;
        	grainIndex[i] = 0;
        	grainOverlap[i].store(0.0f);
        }
    };
    
    ~GranularSynth(){
    };
    
    void nextBlock(float* bufferToWrite, const int blockSize)
    {
        //TODO granularボイス管理再設計
//        const float* ptr = buffer->getReadPtr();
//        for(int sample = 0; sample < blockSize; ++sample) {
//            for(int voice = 0; voice < NumVoice; ++voice) {
//                for(int g = 0; g < NumGrains; ++g) {
//                    if(grains[voice][g].isPlaying) {
//                        //TODO panning
//                        bufferToWrite[sample] += grains[voice][g].processNextSample(ptr);
//                    }
//                    counterSinceLastTrigger[voice]++;
//                    if(counterSinceLastTrigger >= ){}
//                }
//            }
//        }
    }
    
    void triggerNextGrain(const int voiceIndex) 
    {
    	if(voiceIndex >= NumVoice) {
            std::cout<<"Error GranularSynth-triggerNextGrain(): Invalid voiceIndex"<<std::endl;
            return;
        }
        
        
    }
    
    void setGrainSize(const float size, const int voiceIndex)// size 0.0f~1.0f, voiceIndex 0~(numVoice-1)
    {
        if(voiceIndex >= NumVoice) {
            std::cout<<"Error GranularSynth-setGrainSize(): Invalid voiceIndex"<<std::endl;
            return;
        }
        
        if(size < 0.0 || 1.0 < size) {
            std::cout<<"Error GranularSynth-setGrainSize(): Invalid grain size"<<std::endl;
            return;
        }
        grainSize[voiceIndex] = (float)MaxGrainSizeInSamples * size;
    }
    
    void setBufferOnset(const float onset, const int voiceIndex)//onset:0~1
    {
        if(voiceIndex >= NumVoice) {
            std::cout<<"Error GranularSynth-setBufferOnset(): Invalid voiceIndex"<<std::endl;
            return;
        }
        
        if(onset < 0.0 || 1.0 < onset) {
            std::cout<<"Error GranularSynth-setBufferOnset(): Invalid buffer onset"<<std::endl;
            return;
        }
        bufferOnset[voiceIndex] = (float)(buffer->getSize() - MaxGrainSizeInSamples - 1) * onset;
    }
    
    void setOverlap(const float overlap, const int voiceIndex)//overlap:0~1
    {
        if(voiceIndex >= NumVoice) {
            std::cout<<"Error GranularSynth-setOverlap(): Invalid voiceIndex"<<std::endl;
            return;
        }
        
        if(overlap < 0.0 || 1.0 < overlap) {
            std::cout<<"Error GranularSynth-setOverlap(): Invalid buffer position"<<std::endl;
            return;
        }
        
        grainOverlap[voiceIndex] = overlap * (Pi - MinOverlapInPhase) + MinOverlapInPhase;//MinOverlapInPhase~Pi
    }
    
    void loadFile(const std::string audioFileName)
    {
        const int numSamples = getNumFrames(audioFileName);
        if(numSamples < MinBufferLengthInSamples) {
            std::cout<<"Error GranularSynth-loadFile(): Too short sample length"<<std::endl;
            return;
        }
        
        buffer->loadSampleFile(audioFileName);
        for(int i = 0; i < NumVoice; ++i) {
        	bufferOnset[i] = 0;
        }
    }
    
    std::unique_ptr<MonoBuffer> buffer;
    static constexpr int MaxGrainSizeInSamples = 22050;//500mS
    static constexpr int MinBufferLengthInSamples = 35280;//800mS
    
private:
    static constexpr int NumGrains = 16;
    static constexpr int NumVoice = 2;
    static constexpr float MinOverlapInPhase = Pi / NumGrains;
    std::array<std::atomic<int>, NumVoice> bufferOnset;
    std::array<std::atomic<int>, NumVoice> grainSize;
    int grainIndex[NumVoice];
    std::array<std::atomic<float>, NumVoice> grainOverlap;//0~Pi
    int counterSinceLastTrigger[NumVoice];
    
    class Grain
    {
    public:
        Grain(){};
        ~Grain(){};
        
        void trigger(const int bufferOnset, const int grainSize)
        {
        	onset = bufferOnset;
        	phaseStep.store(Pi / (float)grainSize);
        	phase.store(0.0f);
        	isPlaying = true;
        }
        
        float processNextSample(const float* bufferToRead){
        	if(isPlaying == false){
        		return 0.0f;
        	}
        	
        	float ph = phase.load();
        	const float phStep = phaseStep.load();
        	const float output = bufferToRead[onset] * sinWindow(ph);
            onset++;
            ph += phStep;
            phase.store(ph);
            if(ph >= Pi) isPlaying = false;
            return output;
        }
        
        std::atomic<bool> isPlaying{false};
        
    private:
    	inline float sinWindow(float p)//0~Pi
    	{
    		return sinf_neon(p);
    	}
        
        std::atomic<int> onset{0};
        std::atomic<float> phase{0.0f};
        std::atomic<float> phaseStep{0.0f};
    };
    
    Grain grains[NumVoice][NumGrains];
};

#endif /* GranularSynth_H_ */
