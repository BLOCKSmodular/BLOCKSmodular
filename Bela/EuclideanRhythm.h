/*
 EuclideanRhythm.h for BLOCKSmodular
 Created by Akiyuki Okayasu
 License: GPLv3
 */
#ifndef EuclideanRhythm_h
#define EuclideanRhythm_h

#include <string>
#include <bitset>
#include <atomic>
#include <algorithm>

class EuclideanRhythm {
public:
    void init(const int digitalSampleRate)
    {
        sampleRate = digitalSampleRate;
    }
    
    void generateRhythm()
    {
        const int numBeats = numSteps * beatRatio;
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
        for(int i = 0; i < numSteps; ++i) {
            if(seq[i] == '1') {
                rhythmPattern.set(i);
            }else{
                rhythmPattern.reset(i);
            }
        }
    }
    
    bool update(){
        sampleCounter++;
        if(sampleCounter > stepWidthInSamples) {
            stepPosition++;
            if(stepPosition > numSteps) stepPosition = 0;
            sampleCounter = 0;
            return rhythmPattern.test(stepPosition);
        } else if(sampleCounter < GateWidthInSamples) {
            return rhythmPattern.test(stepPosition);
        } else {
            return false;
        }
    }
    
    void setnumSteps(const unsigned int step) {
        if(MaxNumSteps < step) {
            std::cout<<"Error: numSteps must be less than 128"<<std::endl;
            return;
        }
        numSteps = step;
    }
    
    void setBeatRatio(const float ratio) {
        if(ratio < 0.0f || 1.0f < ratio) {
            std::cout<<"Error: beatRatio must be in the range 0~1"<<std::endl;
            return;
        }
        beatRatio.store(ratio);
    }
    
    void setBPM(const float bpm) {
        stepWidthInSamples = bpm / (float)(sampleRate * 60.0f);
    }
    
private:
    static constexpr int MaxNumSteps = 127;//シーケンサーの最大ステップ数
    static constexpr int GateWidthInSamples = 50;//Gate信号ONの長さ(サンプル数)
    std::bitset<MaxNumSteps> rhythmPattern{0};//シーケンスパターン
    std::atomic<int> numSteps{0};//現在のステップ数
    int stepPosition = 0;//シーケンサーのステップ位置
    std::atomic<float> beatRatio{0.0f};//シーケンス内のトリガーする割合(0.0f~1.0f)
    std::atomic<int> stepWidthInSamples{100};//1ステップの長さ（サンプル数）
    int sampleCounter = 0;//1ステップの長さのカウンター
    std::atomic<int> sampleRate;
};

#endif /* EuclideanRhythm_h */
