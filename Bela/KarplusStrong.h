//  KarplusStrong.h
//  Created by Akiyuki Okayasu
#ifndef KarplusStrong_h
#define KarplusStrong_h

#include <math_neon.h>//Fast math function
#include <memory>
#include <string>
#include "SampleBuffer.h"
#include "Util.h"

class KarplusStrong {
public:
    KarplusStrong(){
    	source = std::make_unique<MonoBuffer>(4410, false, false);
    	source->loadSampleFile("KarplusStrongSource.wav");
        source->setReadIter(source->getSize() - 1);//起動時になって1発鳴ってしまうのを防止する
    };
    ~KarplusStrong(){};
    
    void trigger(){
    	sample = 0.0f;
        source->setReadIter(0);
    }
    
    void nextBlock(float* bufferToWrite, const int blockSize){
        for(unsigned int i = 0; i < blockSize; ++i) {
            source->readNext(sample);
            sample = lowpass.process(sample);
            bufferToWrite[i] += sample;
        }
    }
    
    void setFreq(const float freq){
        lowpass.setCutoffFreq(freq);
        lowpass.updateParameter();
    }
    
    void setDecay(const float decay) {
    	lowpass.setDecay(decay);
    }
    
private:
    class LPF {
    public:
        LPF(){};
        ~LPF(){};
        
        float process(const float input){
            const float output = (1.0f - r) * (input - r * x2) + c1 * y1 + c2 * y2;
            y2 = y1;
            y1 = output;
            x2 = x1;
            x1 = input;
            return output;
        }
        
        void setCutoffFreq(const float freq) {
            cutoffFreq.store(freq);
        }
        
        void setDecay(const float decay) {
            if(0.0f < decay && decay <= 1.0f) {
                bandWidth.store((1.0f - decay) * 100.0f + 30.0f);
            }
        }
        
        void updateParameter(){//Must be call this function after setCutoffFreq() or setDecay()
            const float rvalue = exp(-1.0f * bandWidth.load() * normTwoPi);
            r.store(rvalue);
            c1.store(2.0f * rvalue * cosf_neon(cutoffFreq.load() * normTwoPi));
            c2.store((-1.0f * rvalue) * rvalue);
        }
        
    private:
        float x1{0.0f}, x2{0.0f}, y1{0.0f}, y2{0.0f};
        static constexpr float normTwoPi = 6.28318530718f / 44100.0f;
        std::atomic<float> cutoffFreq{200.0f};
        std::atomic<float> bandWidth{0.0f};
        std::atomic<float> r{0.0f};
        std::atomic<float> c1{0.0f};
        std::atomic<float> c2{0.0f};
    };
    
    float sample = 0.0f;
    LPF lowpass;
    std::unique_ptr<MonoBuffer> source;
};

#endif /* KarplusStrong_h */
