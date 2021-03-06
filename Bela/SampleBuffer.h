/*
 SampleBuffer.h for BLOCKSmodular
 Created by Akiyuki Okayasu
 License: GPLv3
 */

#ifndef SAMPLEBUFFER_H_
#define SAMPLEBUFFER_H_

#include <Bela.h>
#include <sndfile.h>// to load audio files
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <Util.h>
#include <atomic>

class MonoBuffer {
public:
    MonoBuffer(const int size, bool loopPlaying = false, bool loopRecording = false)
    :readLoop(loopPlaying), writeLoop(loopRecording)
    {
        buffer.resize(size);
        bufferSize = size;
    }
    
    ~MonoBuffer(){}
    
    void resize(const int size)
    {
        buffer.resize(size, 0.0f);
        bufferSize = size;
    }
    
    void nextBlock(float* bufferToWrite, const int blockSize)
    {
        for(int i = 0; i < blockSize; ++i)
        {
            bufferToWrite[i] += buffer[readIter];
            addReadIter(1);
        }
    }
    
    const float* getReadPtr()
    {
        return buffer.data();
    }
    
    float* getWritePtr()
    {
        return buffer.data();
    }
    
    int getSize()
    {
        return bufferSize;
    }
    
    const unsigned int getWriteIter()
    {
        return writeIter;
    }
    
    const unsigned int getReadIter()
    {
        return readIter;
    }
    
    void readNext(float& value)
    {
        if(readIter < bufferSize)
        {
            value += buffer[readIter];
            addReadIter(1);
        }
    }
    
    void writeNext(float& value)
    {
        buffer[readIter] = value;
        addWriteIter(1);
    }
    
    void addReadIter(const unsigned int add)
    {
        if(readIter + add >= bufferSize && !readLoop) {
            readIter = bufferSize;
            return;
        }
        
        readIter += add;
        if(readIter >= bufferSize) {
            readIter -= bufferSize;
        }
    }
    
    void addWriteIter(const unsigned int add)
    {
        if(writeIter + add >= bufferSize && !writeLoop) {
            writeIter = bufferSize;
            return;
        }
        
        writeIter += add;
        if(writeIter >= bufferSize) {
            writeIter -= bufferSize;
        }
    }
    
    void setReadIter(const unsigned int itr) {
        if(itr < buffer.size()) {
            readIter = itr;
        }
        else{
            std::cout<<"MonoBuffer-setReadIter(): Invalid iterater"<<std::endl;
        }
    }
    
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
        
        const int frameLen = getNumFrames(file);
        if(buffer.size() < frameLen) {
            std::cout << "Warning: MonoBuffer class was resized!!!" << std::endl;
            buffer.resize(frameLen, 0.0f);
            bufferSize = frameLen;
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
    
private:
    std::vector<float> buffer;
    std::atomic<unsigned int> readIter{0};
    std::atomic<unsigned int> writeIter{0};
    unsigned int bufferSize;
    bool readLoop;
    bool writeLoop;
};

//---------------------------------------------------------------------------------------
class StereoBuffer {
public:
    StereoBuffer(const int size = 88400, bool loopPlaying = false, bool loopRecording = false)
    :readLoop(loopPlaying), writeLoop(loopRecording)
    {
        for(int channel = 0; channel < numBufferChannels; ++channel) {
            buffer[channel].resize(size, 0.0f);
        }
        bufferSize = size;
    }
    
    ~StereoBuffer(){}
    
    void resize(const int size)
    {
        for(int channel = 0; channel < numBufferChannels; ++channel) {
            buffer[channel].resize(size, 0.0f);
        }
        bufferSize = size;
    }
    
    void nextBlock(float* LeftBufferToWrite, float* RightBufferToWrite, const int blockSize)
    {
        for(int i = 0; i < blockSize; ++i)
        {
            LeftBufferToWrite[i] += buffer[0][readIter];
            RightBufferToWrite[i] += buffer[1][readIter];
            addReadIter(1);
        }
    }
    
    const float* getReadChannelPtr(const int channel)
    {
        return buffer[channel].data();
    }
    
    float* getWriteChannelPtr(const int channel)
    {
        return buffer[channel].data();
    }
    
    int getSize()
    {
        return bufferSize;
    }
    
    const unsigned int getWriteIter()
    {
        return writeIter;
    }
    
    const unsigned int getReadIter()
    {
        return readIter;
    }
    
    void readNext(float& leftCh, float& rightCh)
    {
        if(readIter < bufferSize)
        {
            leftCh += buffer[0][readIter];
            rightCh += buffer[1][readIter];
            addReadIter(1);
        }
    }
    
    bool isBufferEnd() {
        return readIter >= bufferSize;
    }
    
    void writeNext(const float& leftCh, const float& rightCh)
    {
        buffer[0][readIter] = leftCh;
        buffer[1][readIter] = rightCh;
        addWriteIter(1);
    }
    
    void addReadIter(const unsigned int add)
    {
        if(readIter + add >= bufferSize && !readLoop) {
            readIter = bufferSize;
            return;
        }
        
        readIter += add;
        if(readIter >= bufferSize) {
            readIter -= bufferSize;
        }
    }
    
    void addWriteIter(const unsigned int add)
    {
        if(writeIter + add >= bufferSize && !writeLoop) {
            writeIter = bufferSize;
            return;
        }
        
        writeIter += add;
        if(writeIter >= bufferSize) {
            writeIter -= bufferSize;
        }
    }
    
    void setReadIter(const unsigned int itr) {
        if(itr < bufferSize) {
            readIter = itr;
        }
        else{
            std::cout<<"StereoBuffer-setReadIter(): Invalid iterater"<<std::endl;
        }
    }
    
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
        
        if(numChannelsInFile != 2)//ステレオファイルのみ許可
        {
            std::cout << "Error: " << file << " doesn't contain requested channel" << std::endl;
            return;
        }
        
        const int frameLen = getNumFrames(file);
        if(buffer[0].size() < frameLen) {
            std::cout << "Warning: StereoBuffer class was resized!!!" << std::endl;
            for(int channel = 0; channel < numBufferChannels; ++channel) {
                buffer[channel].resize(frameLen, 0.0f);
            }
            bufferSize = frameLen;
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
        
        for(int n=0;n<frameLen;n++) {
            for(int channel = 0; channel < numBufferChannels; ++channel) {
                buffer[channel][n] = tempBuf[n*numChannelsInFile + channel];
            }
        }
        
        sf_close(sndfile);
    }
    
private:
    static constexpr int numBufferChannels{2};//stereo
    std::vector<float> buffer[numBufferChannels];
    std::atomic<unsigned int> readIter{0};
    std::atomic<unsigned int> writeIter{0};
    unsigned int bufferSize;
    bool readLoop;
    bool writeLoop;
};

#endif /* SAMPLEBUFFER_H_ */
