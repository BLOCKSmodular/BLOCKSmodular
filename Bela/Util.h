/***** Util.h *****/
#ifndef UTIL_H_
#define UTIL_H_

#include <Bela.h>
#include <sndfile.h>// to load audio files
#include <string>
#include <iostream>
#include <cstdlib>

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

struct HighResolutionCC {
    char up = -1;
    char low = -1;
    float value = 0.0f;
    
    bool update() {
        if(up < 0 || low < 0) {
            return false;
        }
        
        value = (float)((up << 7) | (low & 0b1111111)) / 16384.0f;
        return true;
    }
    
};

#endif//Util.h
