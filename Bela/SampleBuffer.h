/***** SampleBuffer.h *****/
#ifndef SAMPLEBUFFER_H_
#define SAMPLEBUFFER_H_

#include <Bela.h>
#include <sndfile.h>				// to load audio files
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>

class SampleBuffer {
public:
	SampleBuffer(const int size)
	{
		buffer.resize(size);
	}
	~SampleBuffer(){
	}
	
	void resize(const int size)
	{
		buffer.resize(size, 0.0f);
	}
	
	float* getPtr()
	{
		return buffer.data();
	}
	
	int getSize()
	{
		return buffer.size();
	}
	
	unsigned int getItr()
	{
		return iter;
	}
	
	float nextValue() 
	{
		float value = buffer[iter];
		addIter(1);
		return value;
	}
	
	void addIter(const unsigned int add)
	{
		iter += add;
		checkIter();
	}
	
	inline void checkIter()
	{
		const int bufSize = buffer.size();
		if(iter >= bufSize) {
			iter -= bufSize;
		}
	}
	
	// Load samples from file
	void loadSampleFile(std::string file)
	{
		SNDFILE *sndfile ;
		SF_INFO sfinfo ;
		sfinfo.format = 0;
		if (!(sndfile = sf_open (file.c_str(), SFM_READ, &sfinfo))) {
			std::cout << "Couldn't open file " << file << ": " << sf_strerror(sndfile) << std::endl;
			return;
		}

		int numChannelsInFile = sfinfo.channels;
		
		if(numChannelsInFile != 1)//1chのファイルのみ許可
		{
			std::cout << "Error: " << file << " doesn't contain requested channel" << std::endl;
			return;
		}
    
    	// int frameLen = endFrame-startFrame;
    	const int frameLen = getNumFrames(file);
    	if(buffer.size() < frameLen) {
    		std::cout << "Warning: SampleBuffer class was resized!!!" << std::endl;
    		buffer.resize(frameLen, 0.0f);
    	}
    	
    	sf_seek(sndfile,0,SEEK_SET);
    	float* tempBuf = new float[frameLen*numChannelsInFile];
		int subformat = sfinfo.format & SF_FORMAT_SUBMASK;
		int readcount = sf_read_float(sndfile, tempBuf, frameLen*getNumChannels(file)); //FIXME

		// Pad with zeros in case we couldn't read whole file
		for(int k = readcount; k <frameLen*numChannelsInFile; k++)
			tempBuf[k] = 0;

		if (subformat == SF_FORMAT_FLOAT || subformat == SF_FORMAT_DOUBLE) 
		{
			double	scale ;
			int 	m ;

			sf_command (sndfile, SFC_CALC_SIGNAL_MAX, &scale, sizeof (scale)) ;
			if (scale < 1e-10)
				scale = 1.0 ;
			else
				scale = 32700.0 / scale ;
			std::cout << "File samples scale = " << scale << std::endl;

			for (m = 0; m < frameLen; m++)
				tempBuf[m] *= scale;
		}
	
		for(int n=0;n<frameLen;n++)
		{
		    buffer[n] = tempBuf[n*numChannelsInFile];
		    /*Streeo buffer and Stereo file
		    buffer[n][0] = tempBuf[n*numChannelsInFile + 0];
		  	buffer[n][1] = tempBuf[n*numChannelsInFile + 1];
		    */
		}

		sf_close(sndfile);
	}

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
	
private:
	std::vector<float> buffer;
	unsigned int iter;
};

#endif /* SAMPLEBUFFER_H_ */


