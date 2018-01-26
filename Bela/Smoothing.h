/***** Smoothing.h *****/
#ifndef SMOOTHING_H_
#define SMOOTHING_H_

#include <stdlib.h>
#include <atomic>

#define SAMPLERATE 44100

struct Smoothing
{
	std::atomic<float> currentValue{0.0f};
	std::atomic<float> step{0.0f};
	std::atomic<int> smoothingIndex{0};
	std::atomic<int> smoothingLength{1200};

	void init(const int numStep)
	{
		smoothingLength.store(numStep);
	}

	void reset()
	{
		step.store(0.0f);
		currentValue.store(0.0f);
		smoothingIndex.store(0);
	}

	void set(float target)
	{
		step.store((target - currentValue.load()) / (float)smoothingLength.load());
		smoothingIndex.store(0);
	}

	float getNextValue()
	{
		if (smoothingIndex.load() < smoothingLength.load())
		{
			currentValue.store(currentValue.load() + step.load());
			++smoothingIndex;
		}
		return currentValue;
	}
};

#endif /* SMOOTHING_H_ */