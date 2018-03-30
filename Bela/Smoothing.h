/***** Smoothing.h *****/
#ifndef SMOOTHING_H_
#define SMOOTHING_H_

#include <stdlib.h>
#include <atomic>

class Smoothing
{
public:
    Smoothing(){}
    ~Smoothing(){}
    
    void set(const float target)
    {
        stepSize = (target - currentValue.load()) / (float)smoothingLength;
        index = 0;
    }
    
    float getNextValue()
    {
        if (index.load() < smoothingLength)
        {
            currentValue = currentValue.load() + stepSize.load();
            index++;
        }
        return currentValue;
    }
    
private:
    std::atomic<float> currentValue{0.0f};
    std::atomic<float> stepSize{0.0f};
    std::atomic<int> index{0};
    static constexpr int smoothingLength = 1200;
};

#endif /* SMOOTHING_H_ */
