//  LogisticMap.h
//  Created by Akiyuki Okayasu
#ifndef LogisticMap_H_
#define LogisticMap_H_

#include <atomic>
#include "Util.h"

class LogisticMap {
public:
    LogisticMap(){
    	smoothGain.set(0.0f);
    };
    ~LogisticMap(){};
    
    float update(){
    	x = alpha.load() * x * (1.0 - x) ;
    	const float g = smoothGain.getNextValue();
    	return (x * 2.0 - 1.0f)  * g;
    };
    
    void setAlpha(double a) {
    	if(0.0 < a && a < 4.0) {
    		alpha.store(a);
    	}
    }
    
    void setGain(float gain) {// gain 0.0f~1.0f
    	smoothGain.set(gain);
    }
    
    
private:
	double x = 0.5;
	std::atomic<double> alpha{3.7};
	Smoothing smoothGain;
};
#endif /* LogisticMap_H_ */
