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
        const float s = twoPi / (float)numGrains;
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
        if(voiceIndex >= numVoice) {
            std::cout<<"Error GranularSynth-setWindowShape(): Invalid voiceIndex"<<std::endl;
            return;
        }
        windowShape[voiceIndex] = intensity;
    }
    
    void setDensity(const float denst, const int voiceIndex)
    {
        if(voiceIndex >= numVoice) {
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
    static constexpr int numGrains = 15;
    static constexpr int numVoice = 2;
    int bufferPosition[numVoice]{0, 0};//TODO: atomic
    int grainSize[numVoice]{10000, 10000};//TODO: atomic
    float windowShape[numVoice]{0.0f, 0.0f};//TODO: atomic
    float density[numVoice]{0.5f, 0.5f};//TODO: atomic
    std::mt19937 random{12345};//TODO: seedの変更
    std::uniform_real_distribution<float> dist{0.0f, 1.0f}; 
    
    bool dice(const int vI) {
    	if(dist(random) <= density[vI]) {
    		return true;
    	}
    	else {
    		return false;
    	}
    }
    
    class Grain
    {
    public:
        Grain(const int voiceIndex, GranularSynth& g)
        : voiceIndex(voiceIndex), granular_(g)
        {};
        ~Grain(){};
        
        void init(const float phase)
        {
        	if(phase < 0.0f || twoPi < phase) std::cout<<"Error Grain-init(): Invalid phase"<<std::endl;
            windowStep = twoPi / (float)granular_.grainSize[voiceIndex];
            windowPhase = phase;
        }
        
        void update(const float* bufferToRead, float* bufferToWrite, const int length){
        	if(windowPhase < twoPi) {
  	            for(int i = 0; i < length; ++i) {
	        		bufferToWrite[i] += bufferToRead[bufferPos] * variableWindow();
               		bufferPos++;
                	windowPhase += windowStep;
                	if(windowPhase >= twoPi) {
                		parameterUpdate();
                		return;
                	}
            	}
        	}
        	else{
        		parameterUpdate();
        	}
        }
        
    private:
        inline float variableWindow()
        {
            /*
            windowShape<=1.0: 0~(Pi * windowShape)の範囲のハン窓
            windowShape>1,0: windowShapeが大きいほど矩形窓に近づいていく
            */
            //TODO: あとで窓関数をグラフにして確認確認する
            return tanhf_neon((halfPi - halfPi * cosf_neon(windowPhase)) * windowShape);
        }
        
        void parameterUpdate()
        {
        	if(granular_.dice(voiceIndex)) {
            	bufferPos = granular_.bufferPosition[voiceIndex];
            	windowShape = granular_.windowShape[voiceIndex];
            	windowStep = twoPi / (float)granular_.grainSize[voiceIndex];
            	windowPhase = 0.0f;
        	}
        }
        
        int voiceIndex;
        int bufferPos = 0;
        float windowShape = 0.0f;
        float windowStep = 0.05f;
        float windowPhase = 0.0f;//0.0f~2pi
        GranularSynth& granular_;
    };
    
    Grain* grains[numVoice][numGrains];
};

#endif /* GranularSynth_H_ */
