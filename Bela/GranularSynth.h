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
    	for(int i = 0; i < numVoice; ++i) {
    		samplePosition[i] = 0;
    	}
    	
        buffer = std::make_unique<MonoBuffer>(44100, false, false);
        random = std::make_unique<std::mt19937>(rand());
        const int s = grainSize.load() / numGrains;
        for(int i = 0; i < numVoice; ++i) {
        	for (int k = 0; k < numGrains; ++k) {
            	grains[i][k] = new Grain(*this, i);
            	grains[i][k]->init(i * s);
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
        for(int i = 0; i < 4; ++i) {
        	for (int k = 0; k < numGrains; ++k) {
         		grains[i][k]->update(ptr, bufferToWrite, blockSize);
        	}
        }
    }
    
    void setGrainSize(const float sizeAmount, const int id_)
    {
    	grainSize[id_] = (float)maxGrainSize * sizeAmount;
    }
    
    void setSamplePosition(const unsigned int pos, const int id)
    {
    	samplePosition[id] = pos;
    }
    
    void loadFile(const std::string audioFileName)
    {
        //TODO 各グレインのwindowPhaseとsampleIndexを変更するのでクリックノイズが発生する可能性がある -> loadFile()直後はフェードインさせる処理を追加
        const int numSamples = getNumFrames(audioFileName);
        if(numSamples < minSampleLength) {
            std::cout<<"Error GranularSynth: Too short sample length"<<std::endl;
        }
        else {
            buffer->loadSampleFile(audioFileName);
            const int s = numSamples / numGrains;
        	for(int i = 0; i < 4; ++i) {
        		samplePosition[i] = s * i;
        		for (int k = 0; k < numGrains; ++k) {
            		grains[i][k]->init(i * s);
        		}
        	}
        }
    }
    
    std::unique_ptr<MonoBuffer> buffer;
    static constexpr int maxGrainSize = 22050;//500mS
    static constexpr int minSampleLength = 35280;//800mS
    
private:
    static constexpr int numGrains = 8;
    static constexpr int numVoice = 4;
    static constexpr float twoPi = 6.28318530718f;
    std::atomic<int> grainSize{maxGrainSize};
    std::unique_ptr<std::mt19937> random;//TODO: シードをdevice_randomで生成する
    std::uniform_real_distribution<float> dist{0.0f, 1.0f};
    int samplePosition[numVoice]{0, 0, 0, 0};//TODO: atomic
    int grainSize[numVoice]{10000, 10000, 10000, 10000};//TODO: atomic
    float amp[numVoice]{1.0f, 1.0f, 1.0f, 1.0f};//TODO: atomic
    
    class Grain
    {
    public:
        Grain(GranularSynth& g, const int id_)
        : granular_(g), voiceID(id_)
        {};
        ~Grain(){};
        
        void init(const unsigned int index)
        {
            currentGrainSize = granular_.grainSize[voiceID];
            
            if(currentGrainSize <= index) {
                std::cout<<"Error GranularSynth: Invalid index"<<std::endl;
                sampleIndex = 0;
                windowStep = twoPi / (float)currentGrainSize;
                windowPhase = 0.0f;
            }
            else {
                sampleIndex = index;
                windowStep = twoPi / (float)currentGrainSize;
                windowPhase = windowStep * (float)index;
            }
        }
        
        void update(const float* bufferToRead, float* bufferToWrite, const int length){
            for(int i = 0; i < length; ++i) {
                bufferToWrite[i] += bufferToRead[startSample + sampleIndex] * window();
                sampleIndex++;
                windowPhase += windowStep;
                if(sampleIndex >= currentGrainSize) {
                    parameterUpdate();
                    return;
                }
            }
        }
        
    private:
        inline float window()
        {
            return 0.5f - 0.5f * cosf_neon(windowPhase);
        }
        
        void parameterUpdate()
        {
            currentGrainSize = granular_.grainSize[voiceID];
            startSample = granular_.samplePosition[voiceID];
            grainAmp = granular_.amp[voiceID];
            windowStep = twoPi / (float)currentGrainSize;
            windowPhase = 0.0f;
            sampleIndex = 0;
        }
        
        int voiceID;
        int startSample = 0;
        int currentGrainSize = 10000;
        int sampleIndex;
        float windowStep;
        float windowPhase;//0.0f~2pi
        GranularSynth& granular_;
    };
    
    Grain* grains[numVoice][numGrains];
};

#endif /* GranularSynth_H_ */
