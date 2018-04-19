//  EuclideanRhythm.h
//  Created by Akiyuki Okayasu
#ifndef EuclideanRhythm_h
#define EuclideanRhythm_h

#include <string>
#include <bitset>
#include <atomic>

class EuclideanRhythm(){
public:
    EuclideanRhythm();
    ~EuclideanRhythm();
    void generateRhythm(const int numSteps, const int numBeats)
    {
        if (numSteps < numBeats) {
            std::cout << "Error: numBeats must be less than numSteps" << std::endl;
            return "0";
        }
        
        if(StepSize > numSteps){
            
        }
        
        std::string x = "1";
        int numX = numBeats;
        std::string y = "0";
        int numY = numSteps - numBeats;
        
        do {
            const std::string tmp_x = x;
            const std::string tmp_y = y;
            const int tmp_numX = numX;
            const int tmp_numY = numY;
            x += tmp_y;
            if (numX > numY) {
                y = tmp_x;
                numX = tmp_numY;
                numY = tmp_numX - tmp_numY;
            }
            else {
                numX = tmp_numX;
                numY = tmp_numY - tmp_numX;
            }
        } while (numX > 1 && numY > 1);
        
        std::string seq;
        for (int i = 0; i < numX; ++i) seq += x;
        for (int i = 0; i < numY; ++i) seq += y;
        
        for(unsigned int i = 0; i < std::min(numSteps, maxStepSize); ++i) {
            if(seq[i] == "1") {
                rhythmPattern.set(i);
            }else{
                rhythmPattern.reset(i);
            }
        }
    }
    
    int getStepSize(){
        return StepSize.load();
    }
    
    bool nextStep(){
        if(currentStep >= StepSize) {
            currentStep = 0;
        }
        
        return rhythmPattern.test(currentStep);
    }
    
private:
    static constexpr int maxStepSize = 32;
    std::bitset<maxStepSize> rhythmPattern{0};
    std::atomic<int> StepSize{0};
    std::atomic<int> currentStep{0};
};

#endif /* EuclideanRhythm_h */
