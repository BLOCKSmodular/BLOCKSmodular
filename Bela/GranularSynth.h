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
		random = std::make_unique<std::mt19937>(rand());
		
		for (int i = 0; i < numGrains; ++i)
		{
			grains[i] = new Grain(*this);
		}
		
		buffer = std::make_unique<MonoBuffer>(44100, false, false);
		for(int i = 0; i < numGrains; ++i)
		{
			const int s = grainSize.load() / numGrains;
			grains[i]->init(i * s);
		}
	};
	
	~GranularSynth(){
		for(int i = 0; i < numGrains; ++i)
		{
			delete grains[i];
		}
	};
	
	void nextBlock(float* bufferToWrite, const int blockSize)
	{
		const float* ptr = buffer->getReadPtr();
		for(int i = 0; i < numGrains; ++i)
		{
			grains[i]->update(ptr, bufferToWrite, blockSize);
		}
	}
	
	void setGrainsSize(const unsigned int& grainSizeInSamples)
	{
		if(grainSizeInSamples > maxGrainSize)
		{
			std::cout<<"Error GranularSynth: Too long grain size"<<std::endl;
		}
		else
		{
			grainSize = grainSizeInSamples;
		}
	}
	
	void setSampleRange(const unsigned int& min, const unsigned int& Max)
	{
		if(Max - min < maxGrainSize)
		{
			std::cout<<"Error GranularSynth: Too short SampleRange"<<std::endl;
		}
		else
		{
			dist.param(std::uniform_int_distribution<>::param_type(min, Max - maxGrainSize));
		}
	}
	
	void loadFile(const std::string audioFileName)
	{
		//ファイル読み込み
		const int numSamples = getNumFrames(audioFileName);
		if(numSamples < minSampleLength)
		{
			std::cout<<"Error GranularSynth: Too short sample length"<<std::endl;
		}
		else
		{
			buffer->loadSampleFile(audioFileName);
			setSampleRange(0, numSamples - 1);
		}
	}
	
	void loadBuffer(const float* bufferToRead, const int bufferLength)
	{
		//バッファー読み込み
	}
	
	std::unique_ptr<MonoBuffer> buffer;
	static constexpr int maxGrainSize = 22050;//500mS
	static constexpr int minSampleLength = 35280;//800mS
	
private:
	static constexpr int numGrains = 32;
	static constexpr float twoPi = 6.28318530718f;
	std::atomic<int> grainSize{10000};
	std::unique_ptr<std::mt19937> random;//TODO: シードをdevice_randomで生成する
	std::uniform_int_distribution<> dist{0, 22050};
	
	class Grain
	{
	public:
		Grain(GranularSynth& g)
		: granular_(g)
		{};
		~Grain(){};
		
		void init(const unsigned int index)
		{
			currentGrainSize = granular_.grainSize;
			
			if(currentGrainSize <= index)
			{
				//TODO assertion追加
				std::cout<<"warnig:: invalid index..."<<std::endl;
				sampleIndex = 0;
				windowStep = 1.0f / (float)currentGrainSize;
				windowPhase = 0.0f;
			}
			else
			{
				sampleIndex = index;
				windowStep = 1.0f / (float)currentGrainSize;
				windowPhase = windowStep * (float)index;
			}
		}
		
		void update(const float* bufferToRead, float* bufferToWrite, const int length){
			for(int i = 0; i < length; ++i)
			{
				bufferToWrite[i] += bufferToRead[startSample + sampleIndex] * window();
				sampleIndex++;
				windowPhase = std::min(std::max(0.0f, windowPhase + windowStep), 1.0f);//0.0f~1.0fの範囲
				if(sampleIndex >= currentGrainSize)
				{
					parameterUpdate();
					return;
				}
			}
		}
		
	private:
		inline float window()
		{
			return 0.5f - 0.5f * cosf_neon(twoPi * windowPhase);
		}
	
		void parameterUpdate() 
		{
			windowStep = 1.0f / (float)currentGrainSize;
			currentGrainSize = granular_.grainSize;
			startSample = granular_.dist(*granular_.random);
			windowPhase = 0.0f;
			sampleIndex = 0;
		}
	
		int startSample = 0;
		int currentGrainSize = 10000;
		int sampleIndex;
		float windowStep;
		float windowPhase;//0.0f~1.0f
		GranularSynth& granular_;
	};
	
	Grain* grains[numGrains];
};

#endif /* GranularSynth_H_ */