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
	GranularSynth() {
		buffer = std::make_unique<MonoBuffer>(initialBufferLength, false, false);
		for(int i = 0; i < numGrains; ++i)
		{
			grains[i].init(1, initialBufferLength / 2 - 1, 0, initialBufferLength / 2);
		}
	};
	~GranularSynth(){};
	
	// void loadSample(std::string fileName)
	// {
	// 	//Wavファイル読み込み
	// 	buffer->loadSampleFile(fileName);
	// }
	
	void nextBlock(float* bufferToWrite, const int blockSize)
	{
		const float* ptr = buffer->getReadPtr();
		for(int i = 0; i < numGrains; ++i)
		{
			grains[i].update(ptr, bufferToWrite, blockSize);
		}
	}
	
	void setGrainsSizeRange(const int min, const int Max)
	{
		for(int i = 0; i < numGrains; ++i)
		{
			grains[i].grainSizeRange.first = min;
			grains[i].grainSizeRange.second = Max;
		}
	}
	
	void setStartSampleRange(const int min, const int Max)
	{
		for(int i = 0; i < numGrains; ++i)
		{
			grains[i].startSampleRange.first = min;
			grains[i].startSampleRange.second = Max;
		}
	}
	
	std::unique_ptr<MonoBuffer> buffer;
	
private:
	const int initialBufferLength = 1000;
	static const int numGrains = 16;
	static constexpr float twoPi = 6.283185f;
	
	class Grain
	{
	public:
		Grain(){};
		~Grain(){};
		
		void init(const int grainSizeMin, const int grainSizeMax, const int startSampleMin, const int startSampleMax)
		{
			grainSizeRange.first = grainSizeMin;
			grainSizeRange.second = grainSizeMax;
			startSampleRange.first = startSampleMin;
			startSampleRange.second = startSampleMax;
			parameterUpdate();
		}
		
		void update(const float* bufferToRead, float* bufferToWrite, const int length){
			for(int i = 0; i < length; ++i)
			{
				bufferToWrite[i] += bufferToRead[startSample + sampleIndex] * window();
				sampleIndex++;
				windowPhase += windowStep;
				if(sampleIndex >= grainSize)
				{
					parameterUpdate();
					return;
				}
			}
		}
		
		std::pair<int, int> startSampleRange{0, 0};
		std::pair<int , int> grainSizeRange{0, 0};
		
	private:
		inline float window()
		{
			return 0.5f - 0.5f * cosf_neon(twoPi * windowPhase);
		}
	
		void parameterUpdate() 
		{
			// std::random_device seed;
			std::mt19937 random(rand());//TODO: シードをdevice_randomで生成する
			std::uniform_int_distribution<> gsRandom(grainSizeRange.first, grainSizeRange.second);
			grainSize = gsRandom(random);
			std::uniform_int_distribution<> ssRandom(startSampleRange.first, startSampleRange.second);
			startSample = ssRandom(random);
			// std::cout<<"startSample: "<<startSample<<", grainSize: "<<grainSize<<std::endl;
					
			windowStep = 1.0f / (float)grainSize;
			windowPhase = 0.0f;
			sampleIndex = 0;
		}
	
		int startSample = 0;
		int grainSize;
		int sampleIndex = 0;
		float windowStep = 0.0f;
		float windowPhase = 0.0f;//0.0f~1.0f
	};
	
	Grain grains[numGrains];
};

#endif /* GranularSynth_H_ */