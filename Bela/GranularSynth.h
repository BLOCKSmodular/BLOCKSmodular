/***** GranularSynth.h *****/
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
        const float s = Pi / (float)numGrains;
        for(int i = 0; i < numVoice; ++i) {
            for (int k = 0; k < numGrains; ++k) {
                grains[i][k] = new Grain(i, *this);
                float ph = (float)i * s;
                grains[i][k]->init(ph);
            }
        }
    };
    
    ~GranularSynth(){
        for(int i = 0; i < 4; ++i) {
            for (int k = 0; k < numGrains; ++k) {
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
    
    void setGrainSize(const float size, const int id_)
    {
    	// size 0.0f~1.0f
        grainSize[id_] = (float)maxGrainSize * size;
    }
    
    void setBufferPosition(const float pos, const int id_)
    {
    	// pos 0.0f~1.0f
        bufferPosition[id_] = (float)(buffer->getSize() - maxGrainSize - 1) * pos;
    }
    
    void setWindowShape(const float intensity, const int id_)
    {
    	// intensity 0.0f~1.0f
        windowShape[id_] = intensity;
    }
    
    void loadFile(const std::string audioFileName)
    {
        const int numSamples = getNumFrames(audioFileName);
        if(numSamples < minSampleLength) {
            std::cout<<"Error GranularSynth: Too short sample length"<<std::endl;
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
    static constexpr int numGrains = 6;
    static constexpr int numVoice = 4;
    int bufferPosition[numVoice]{0, 0, 0, 0};//TODO: atomic
    int grainSize[numVoice]{10000, 10000, 10000, 10000};//TODO: atomic
    float windowShape[numVoice]{0.0f, 0.0f, 0.0f, 0.0f};//TODO: atomic
    
    class Grain
    {
    public:
        Grain(const int id_, GranularSynth& g)
        : voiceID(id_), granular_(g)
        {};
        ~Grain(){};
        
        void init(const float phase)
        {
        	if(phase < 0.0f || Pi < phase) std::cout<<"Error GranularSynth: Invalid phase"<<std::endl;
            currentGrainSize = granular_.grainSize[voiceID];
            windowStep = twoPi / (float)currentGrainSize;
            windowPhase = phase;
        }
        
        void update(const float* bufferToRead, float* bufferToWrite, const int length){
            for(int i = 0; i < length; ++i) {
                bufferToWrite[i] += tanhf_neon(bufferToRead[bufferPos] * window() * twoPi);
                bufferPos++;
                windowPhase += windowStep;
                if(windowPhase >= twoPi) {
                    parameterUpdate();
                    return;
                }
            }
        }
        
    private:
        inline float window()
        {
            return (0.5f - 0.5f * cosf_neon(windowPhase)) * gain;
        }
        
        void parameterUpdate()
        {
            currentGrainSize = granular_.grainSize[voiceID];
            bufferPos = granular_.bufferPosition[voiceID];
            gain = granular_.windowShape[voiceID];
            windowStep = twoPi / (float)currentGrainSize;
            windowPhase = 0.0f;
            // rt_printf("ID:%d, size:%d, pos:%d, gain:%f\n", voiceID, currentGrainSize, bufferPos, gain);
        }
        
        int voiceID;
        int bufferPos = 0;
        int currentGrainSize = 10000;
        float gain = 0.0f;
        float windowStep = 0.05f;
        float windowPhase = 0.0f;//0.0f~2pi
        GranularSynth& granular_;
    };
    
    Grain* grains[numVoice][numGrains];
};

#endif /* GranularSynth_H_ */
