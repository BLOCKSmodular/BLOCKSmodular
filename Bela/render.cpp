#include <Bela.h>
#include <Midi.h>
#include <stdlib.h>
#include <cmath>
#include <atomic>
#include <vector>
#include <random>
#include <Smoothing.h>
#include <SampleBuffer.h>
#include <Util.h>
#include <GranularSynth.h>

static constexpr int NUMCVOUT = 8;
static constexpr unsigned char CVModeA =   0b00000001;
static constexpr unsigned char CVModeB =   0b00000010;
static constexpr unsigned char CVModeOFF = 0b00000100;
static constexpr unsigned char CVModeC =   0b00001000;
static constexpr unsigned char CVModeD =   0b00010000;
static constexpr unsigned char AudioModeA =   0b00000001;
static constexpr unsigned char AudioModeB =   0b00000010;
static constexpr unsigned char AudioModeOFF = 0b00000100;
static constexpr unsigned char AudioModeC =   0b00001000;
static constexpr unsigned char AudioModeD =   0b00010000;
unsigned char CVmodeFlag;
unsigned char AudiomodeFlag;
Midi midi;
const char *gMidiPort0 = "hw:1,0,0";
MonoBuffer monoBuffer(88200, true, false);
StereoBuffer stereoBuffer(88200, true, false);
GranularSynth granular;
HighResolutionCC grA_Position;
HighResolutionCC grA_GrainSize;
HighResolutionCC grA_WindowShape;


void midiMessageCallback(MidiChannelMessage message, void *arg)
{
    if (!strcmp(MidiChannelMessage::getTypeText(message.getType()), "control change"))
    {
        const int ccNumber = message.getDataByte(0);
        const int value = message.getDataByte(1);
        // rt_printf("midi: %d, : %d\n", ccNumber, value);
        
        //TODO モード判定追加
        
        if(ccNumber == 1) {
            grA_Position.up = value;
        }
        else if(ccNumber == 2) {
            grA_Position.low = value;
        }
        
        if(ccNumber == 3) {
            grA_GrainSize.up = value;
        }
        else if(ccNumber == 4) {
            grA_GrainSize.low = value;
        }
        
        if(ccNumber == 5) {
            grA_WindowShape.up = value;
        }
        else if(ccNumber == 6) {
            grA_WindowShape.low = value;
        }
        
        if(grA_Position.update()) {
            granular.setBufferPosition(grA_Position.value, 0);
            granular.setBufferPosition(grA_Position.value, 1);
            granular.setBufferPosition(grA_Position.value, 2);
            granular.setBufferPosition(grA_Position.value, 3);
        }
        
        if(grA_GrainSize.update()) {
            granular.setGrainSize(grA_GrainSize.value, 0);
            granular.setGrainSize(grA_GrainSize.value, 1);
            granular.setGrainSize(grA_GrainSize.value, 2);
            granular.setGrainSize(grA_GrainSize.value, 3);
        }
        
        if(grA_WindowShape.update()) {
            const float g = grA_WindowShape.value * 2.0f;
            granular.setWindowShape(g, 0);
            granular.setWindowShape(g, 1);
            granular.setWindowShape(g, 2);
            granular.setWindowShape(g, 3);
        }
    }
}

bool setup(BelaContext *context, void *userData)
{
    //Sleep for waiting boot BLOCKS
    std::cout<<"Begin sleep"<<std::endl;
    sleep(2);
    std::cout<<"End sleep"<<std::endl;
    
    CVmodeFlag = CVModeA;
    AudiomodeFlag = AudioModeA;
    
    //Digital pins setup
    pinMode(context, 0, P8_07, INPUT);//AudioModeA
    pinMode(context, 0, P8_08, INPUT);//AudioModeB
    pinMode(context, 0, P8_09, INPUT);//AudioModeOFF
    pinMode(context, 0, P8_10, INPUT);//AudioModeC
    pinMode(context, 0, P8_11, INPUT);//AudioModeD
    pinMode(context, 0, P8_18, INPUT);//CVModeA
    pinMode(context, 0, P8_27, INPUT);//CVModeB
    pinMode(context, 0, P8_28, INPUT);//CVModeOFF
    pinMode(context, 0, P8_29, INPUT);//CVModeC
    pinMode(context, 0, P8_30, INPUT);//CVModeD
    
    //MIDI
    midi.readFrom(gMidiPort0);
    midi.writeTo(gMidiPort0);
    midi.enableParser(true);
    midi.setParserCallback(midiMessageCallback, (void *)gMidiPort0);
    
    midi_byte_t bytes[3] = {176, (midi_byte_t)(97), 127};
    midi.writeOutput(bytes, 3);
    
    //Load Sample
    monoBuffer.loadSampleFile("vibe.wav");
    stereoBuffer.loadSampleFile("test.wav");
    granular.loadFile("GranularSource.wav");
    
    return true;
}

void render(BelaContext *context, void *userData)
{
    unsigned char cvFLG = 0;
    unsigned char audioFLG = 0;
    //TODOチャタリング除去
    if(digitalRead(context, 0, P8_07)) cvFLG = CVModeA;
    if(digitalRead(context, 0, P8_08)) cvFLG = CVModeB;
    if(digitalRead(context, 0, P8_09)) cvFLG = CVModeOFF;//P8_09はOFFスイッチ
    if(digitalRead(context, 0, P8_10)) cvFLG = CVModeC;
    if(digitalRead(context, 0, P8_11)) cvFLG = CVModeD;
    if(CVmodeFlag != cvFLG && cvFLG != 0) {
        //TODO CVModeリセット関数を追加
        //sendMIDItoCVModeReset();
        
        if(cvFLG == CVModeA) {
            //CVmode1
            //モード切り替え用CC送信
            //midi_byte_t bytes[3] = {176, (midi_byte_t)(97), 127};
            //midi.writeOutput(bytes, 3);
        }
        else if(cvFLG == CVModeB) {
            //CVmode2
            //モード切り替え用CC送信
        }
        else if(cvFLG == CVModeOFF) {
            //CVmode OFF
            //モード切り替え用CC送信
        }
        else if(cvFLG == CVModeC) {
            //CVmode3
            //モード切り替え用CC送信
        }
        else if(cvFLG == CVModeD) {
            //CVmode4
            //モード切り替え用CC送信
        }
        CVmodeFlag = cvFLG;
    }
    
    //-----------------------------------------------------------
    //Digital
    if(digitalRead(context, 0, P8_18)) audioFLG = AudioModeA;
    if(digitalRead(context, 0, P8_27)) audioFLG = AudioModeB;
    if(digitalRead(context, 0, P8_28)) audioFLG = AudioModeOFF;//P8_28はOFFスイッチ
    if(digitalRead(context, 0, P8_29)) audioFLG = AudioModeC;
    if(digitalRead(context, 0, P8_30)) audioFLG = AudioModeD;
    if(AudiomodeFlag != audioFLG && audioFLG != 0) {
        //TODO audioModeリセット関数を追加
        //sendMIDItoAudioModeReset();
        
        if(audioFLG == AudioModeA) {
            //Audiomode1
            //モード切り替え用CC送信
            //midi_byte_t bytes[3] = {176, (midi_byte_t)(97), 127};
            //midi.writeOutput(bytes, 3);
        }
        else if(audioFLG == AudioModeB) {
            //Audiomode2
            //モード切り替え用CC送信
        }
        else if(audioFLG == AudioModeOFF) {
            //Audiomode OFF
            //モード切り替え用CC送信
        }
        else if(audioFLG == AudioModeC) {
            //Audiomode3
            //モード切り替え用CC送信
        }
        else if(audioFLG == AudioModeD) {
            //Audiomode4
            //モード切り替え用CC送信
        }
        AudiomodeFlag = audioFLG;
    }
    
    //-----------------------------------------------------------
    //Analogue
    const int numAnalogueFrames = context->analogFrames;
    switch(CVmodeFlag) {
        case CVModeA: {
            //TODO BLOCKSからの入力に応じた値を出す
            for (unsigned int n = 0; n < numAnalogueFrames; n++) {
                for (unsigned int i = 0; i < NUMCVOUT; i++) {
                    analogWrite(context, n, i, 1.0f);
                }
            }
            break;
        }
        case CVModeB: {
            break;
        }
        case CVModeOFF: {
            break;
        }
        case CVModeC: {
            break;
        }
        case CVModeD: {
            break;
        }
        default: {
            rt_printf("CVMode: %d\n", CVmodeFlag);
            break;
        }
    }
    
    
    //-----------------------------------------------------------
    //Audio
    const int numAudioFrames = context->audioFrames;
    switch(AudiomodeFlag) {
        case AudioModeA: {
            //Granular
            float gr[numAudioFrames];
            for(unsigned int i = 0; i < numAudioFrames; ++i) {
                gr[i] = 0.0f;
            }
            granular.nextBlock(gr, numAudioFrames);
            
            for(unsigned int i = 0; i < numAudioFrames; ++i) {
                audioWrite(context, i, 0, gr[i] * 0.01f);
                audioWrite(context, i, 1, gr[i] * 0.01f);
            }
            break;
        }
        case AudioModeB: {
            //MonoBuffer
            float mono = 0.0f;
            for(unsigned int i = 0; i < numAudioFrames; ++i) {
                monoBuffer.readNext(mono);
                audioWrite(context, i, 0, mono);
                audioWrite(context, i, 1, mono);
            }
            break;
        }
        case AudioModeOFF: {
            break;
        }
        case AudioModeC: {
            //StereoBuffer
            float stereo[2] = {0.0f, 0.0f};
            for(unsigned int i = 0; i < numAudioFrames; ++i) {
                stereoBuffer.readNext(stereo[0], stereo[1]);
                audioWrite(context, i, 0, stereo[0]);
                audioWrite(context, i, 1, stereo[1]);
            }
            break;
        }
        case AudioModeD: {
            break;
        }
        default: {
            rt_printf("AudioMode: %d\n", AudiomodeFlag);
            break;
        }
    }
}

void cleanup(BelaContext *context, void *userData)
{
}
