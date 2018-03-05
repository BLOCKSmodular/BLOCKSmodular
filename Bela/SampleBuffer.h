/***** SampleBuffer.h *****/

#ifndef SAMPLEBUFFER_H_
#define SAMPLEBUFFER_H_

#include <memory>

class SampleBuffer {
public:
	SampleBuffer(int size){
		bufferLength = size;
		buffer.reset(new float[size]);
		for(int i = 0; i < size; ++i) {
			buffer[i] = 0.0f;
		}
	}
	~SampleBuffer(){
		buffer.release();
	}
	
	void resize(int size)
	{
		bufferLength = size;
		buffer.reset(new float[size]);
		for(int i = 0; i < size; ++i) {
			buffer[i] = 0.0f;
		}
	}
	
	float* getBuffer(){
		return buffer.get();
	}
	
	int getSize(){
		return bufferLength;
	}
	
private:
	std::unique_ptr<float[]> buffer;
	int bufferLength;//Buffer length in samples
};

#endif /* SAMPLEBUFFER_H_ */
