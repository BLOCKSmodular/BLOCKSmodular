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
		
		for (int i = 0; i < numGrains; ++i)
		{
			grains[i] = new Grain(*this);
		}
		
		buffer = std::make_unique<MonoBuffer>(44100, false, false);
		const int initialGrainSize = 2400;
		for(int i = 0; i < numGrains; ++i)
		{
			int s = initialGrainSize / numGrains;
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
	
	void setGrainsSize(const int grainSizeInSamples)
	{
		for(int i = 0; i < numGrains; ++i)
		{
			grains[i].nextGrainSize.store(grainSizeInSamples);
		}
	}
	
	void setSampleRange(const int min, const int Max)
	{
		for(int i = 0; i < numGrains; ++i)
		{
			grains[i].sampleRange.first = min;
			grains[i].sampleRange.second = Max;
		}
	}
	
	void loadFile(const std::string audioFileName)
	{
		//ファイル読み込み
		buffer->loadSampleFile(audioFileName);
		setSampleRange(0, getNumFrames("vibe.wav") - 1);
	}
	
	void loadBuffer(const float* bufferToRead, const int bufferLength)
	{
		//バッファー読み込み
	}
	
	std::unique_ptr<MonoBuffer> buffer;
	
private:
	static const int numGrains = 32;
	static constexpr float twoPi = 6.28318530718f;
	
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
		
		std::pair<int, int> sampleRange{0, 22500};
		std::atomic<int> nextGrainSize{10000};
		
	private:
		inline float window()
		{
			return 0.5f - 0.5f * cosf_neon(twoPi * windowPhase);
		}
	
		void parameterUpdate() 
		{
			std::mt19937 random(rand());//TODO: シードをdevice_randomで生成する
			currentGrainSize = nextGrainSize.load();
			
			//TODO範囲外アクセスしないように調整する
			if((sampleRange.second - currentGrainSize) <= sampleRange.first)
			{
				
			}
			std::uniform_int_distribution<> ssRandom(sampleRange.first, sampleRange.second);
			startSample = ssRandom(random);
			windowStep = 1.0f / (float)currentGrainSize;
			currentGrainSize = granular_.grainSize;
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