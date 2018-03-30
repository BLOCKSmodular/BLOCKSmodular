/***** Util.h *****/
#ifndef UTIL_H_
#define UTIL_H_

#include <Bela.h>
#include <sndfile.h>// to load audio files
#include <string>
#include <iostream>
#include <cstdlib>

static constexpr float twoPi = 6.28318530718f;
static constexpr float Pi = 3.14159265359f;

static inline int getNumChannels(std::string file)
{
    SNDFILE *sndfile ;
    SF_INFO sfinfo ;
    sfinfo.format = 0;
    if (!(sndfile = sf_open (file.c_str(), SFM_READ, &sfinfo))) 
    {
        std::cout << "Couldn't open file " << file << ": " << sf_strerror(sndfile) << std::endl;
        return -1;
    }
    return sfinfo.channels;
}

static inline int getNumFrames(std::string file)
{
    SNDFILE *sndfile ;
    SF_INFO sfinfo ;
    sfinfo.format = 0;
    if (!(sndfile = sf_open (file.c_str(), SFM_READ, &sfinfo)))
    {
        std::cout << "Couldn't open file " << file << ": " << sf_strerror(sndfile) << std::endl;
        return -1;
    }
    return sfinfo.frames;
}

class HighResolutionControlChange {
public:
	void setUpperByte(char v) {
		up = v;
	}
	
	void setLowerByte(char v) {
		low = v;
	}
    
    bool update() {
        if(up < 0 || low < 0) {
            return false;
        }
        
        value = (float)((up << 7) | (low & 0b1111111)) / 16384.0f;
        return true;
    }
    
    float getValue() {
    	return value;
    }
    
private:
    char up = -1;
    char low = -1;
    float value = 0.0f;
};

#endif//Util.h
