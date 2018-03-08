/***** Util.h *****/
#ifndef UTIL_H_
#define UTIL_H_

#include <Bela.h>
#include <sndfile.h>// to load audio files
#include <string>
#include <iostream>
#include <cstdlib>

inline int getNumChannels(std::string file) 
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

inline int getNumFrames(std::string file) 
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

#endif//Util.h