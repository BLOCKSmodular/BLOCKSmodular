//  SineCircleMap.h
//  Created by Akiyuki Okayasu
#ifndef SineCircleMap_H_
#define SineCircleMap_H_

#include <math_neon.h>//Fast math function
#include <math.h>
#include <atomic>
#include "Util.h"

class SineCircleMap {
public:
    SineCircleMap(){
    	smoothGain.set(0.0f);
    };
    ~SineCircleMap(){};
    
    float update(){
    	x = fmod(x + o - (k.load() / twoPi * sinf_neon(twoPi * x)), 1.0);
    	return x * smoothGain.getNextValue();
    };
    
    void setK(double value) {// k 1.0 ~1.2f
    	if(1.0 < value && value < 1.2) {
    		k.store(value);
    	}
    }
    
    void setGain(float gain) {// gain 0.0f~1.0f
    	smoothGain.set(gain);
    }
    
private:
	double x = 0.5;
	const double o{0.33};
	std::atomic<double> k{0.5};
	Smoothing smoothGain;
};
#endif /* SineCircleMap_H_ */
