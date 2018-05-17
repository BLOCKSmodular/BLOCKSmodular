/*
 GranularSynth.h for BLOCKSmodular
 Created by Akiyuki Okayasu
 License: GPLv3
 */
#ifndef GranularSynth_H_
#define GranularSynth_H_

#include <math_neon.h>//Fast math function
#include <memory>
#include <array>
#include <vector>
#include <random>
#include <string>
#include "SampleBuffer.h"
#include "Util.h"

class GranularSynth {
public:
    GranularSynth()
    {
        buffer = std::make_unique<MonoBuffer>(44100, false, false);
        const float s = twoPi / (float)numGrains;
        for(int i = 0; i < NumVoice; ++i) {
            for (int k = 0; k < NumGrains; ++k) {
                grains[i][k] = new Grain(i, *this);
                float ph = (float)i * s;
                grains[i][k]->init(ph);
            }
        }
    };
    
    ~GranularSynth(){
        for(int i = NumVoice - 1; i >= 0; --i) {
            for(int k = NumGrains - 1; k >= 0; --k) {
                delete grains[i][k];
            }
        }
    };
    
    void nextBlock(float* bufferToWrite, const int blockSize)
    {
        const float* ptr = buffer->getReadPtr();
        for(int i = 0; i < numVoice; ++i) {
            for (int k = 0; k < numGrains; ++k) {
                grains[i][k]->update(ptr, bufferToWrite, blockSize);
            }
        }
    }
    
    void setGrainSize(const float size, const int voiceIndex)// size 0.0f~1.0f, voiceIndex 0~(numVoice-1)
    {
        if(voiceIndex >= numVoice) {
            std::cout<<"Error GranularSynth-setGrainSize(): Invalid voiceIndex"<<std::endl;
            return;
        }
        
        if(size < 0.0 || 1.0 < size) {
            std::cout<<"Error GranularSynth-setGrainSize(): Invalid grain size"<<std::endl;
            return;
        }
        grainSize[voiceIndex] = (float)maxGrainSize * size;
    }
    
    void setBufferPosition(const float position, const int voiceIndex)// position 0.0f~1.0f, voiceIndex 0~(numVoice-1)
    {
        if(voiceIndex >= numVoice) {
            std::cout<<"Error GranularSynth-setBufferPosition(): Invalid voiceIndex"<<std::endl;
            return;
        }
        
        if(position < 0.0 || 1.0 < position) {
            std::cout<<"Error GranularSynth-setBufferPosition(): Invalid buffer position"<<std::endl;
            return;
        }
        bufferPosition[voiceIndex] = (float)(buffer->getSize() - maxGrainSize - 1) * position;
    }
    
    void setWindowShape(const float intensity, const int voiceIndex)// intensity 0.0f~1.0f, voiceIndex 0~(numVoice-1)
    {
        if(voiceIndex >= NumVoice) {
            std::cout<<"Error GranularSynth-setWindowShape(): Invalid voiceIndex"<<std::endl;
            return;
        }
        windowShape[voiceIndex] = intensity;
    }
    
    void setDensity(const float denst, const int voiceIndex)
    {
        if(voiceIndex >= NumVoice) {
            std::cout<<"Error GranularSynth-setDensity(): Invalid voiceIndex"<<std::endl;
            return;
        }
        
        density[voiceIndex] = denst;
    }
    
    void loadFile(const std::string audioFileName)
    {
        const int numSamples = getNumFrames(audioFileName);
        if(numSamples < minSampleLength) {
            std::cout<<"Error GranularSynth-loadFile(): Too short sample length"<<std::endl;
        }
        else {
            buffer->loadSampleFile(audioFileName);
            const float s = Pi / (float)numGrains;
            for(int i = 0; i < numVoice; ++i) {
                bufferPosition[i] = 0;
                for (int k = 0; k < numGrains; ++k) {
                    float ph = (float)i * s;
                    grains[i][k]->init(ph);
                }
            }
        }
    }
    
    std::unique_ptr<MonoBuffer> buffer;
    static constexpr int maxGrainSize = 22050;//500mS
    static constexpr int minSampleLength = 35280;//800mS
    
private:
    static constexpr int NumGrains = 15;
    static constexpr int NumVoice = 2;
    int bufferPosition[NumVoice]{0, 0};//TODO: atomic
    int grainSize[NumVoice]{10000, 10000};//TODO: atomic
    float windowShape[NumVoice]{0.0f, 0.0f};//TODO: atomic
    
    
    class Grain
    {
    public:
        Grain(const int voiceIndex, GranularSynth& g)
        : voiceIndex(voiceIndex), granular_(g)
        {};
        ~Grain(){};
        
        void init(const float phase)
        {
            if(phase < 0.0f || Pi < phase) std::cout<<"Error Grain-init(): Invalid phase"<<std::endl;
             windowStep = Pi / (float)granular_.grainSize[voiceIndex];
            windowPhase = phase;
        }
        
        void update(const float* bufferToRead, float* bufferToWrite, const int length){
        	if(windowPhase > Pi) parameterUpdate();
        	for(int i = 0; i < length; ++i) {
        		bufferToWrite[i] += bufferToRead[bufferPos] * sinWindow();
                bufferPos++;
                windowPhase += phaseStep;
            }
        }
        
    private:
    	inline float sinWindow()
    	{
    		return sinf_neon(windowPhase);
    	}
        
        void parameterUpdate()
        {
            if(granular_.dice(voiceIndex)) {
                bufferPos = granular_.bufferPosition[voiceIndex];
                windowShape = granular_.windowShape[voiceIndex];
                phaseStep = Pi / (float)granular_.grainSize[voiceIndex];
                windowPhase = 0.0f;
            }
        }
        
        int voiceIndex;
        int bufferPos = 0;
        float phaseStep = 0.01f;
        float windowPhase = 0.0f;//0.0f~Pi
        GranularSynth& granular_;
    };
    
    Grain* grains[numVoice][numGrains];
};

#endif /* GranularSynth_H_ */
