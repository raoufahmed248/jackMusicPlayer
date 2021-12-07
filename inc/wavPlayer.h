
#ifndef wavPlayer_h
#define wavPlayer_h

//#include "dr_wav.cpp"
#include "dr_wav.cpp"
#include "concurrentqueue.h"
#include "wavPlayerTypes.h"
#include <vector>
#include <algorithm>  
//template<size_t frameSize>
class wavPlayer {
public:
    int returnFour();
    wavPlayer(std::string wavFilePath, int sampleDelayBetweenPlays);
    ~wavPlayer();
    // Begin parsing frames until queueSize is reached, returns frames parsed
    int decode();
    // Parse only input amount of frames, returns frames parsed
    int decode(int amount);
    // Get current amount of frames in queue
    int currentQueueSize();
    // Get value of current sample index
    int currentSampleIndex();
    // Get value of total number of sampels in wav file
    int totalSamples();

    // Reset sample index
    void resetIndex();

    //Retrieve samples from left and right channels.
    //NOTE:THIS FUNCTION NEEDS TO ENSURE THERE ARE SAMPLES FOR BOTH LEFT AND RIGHT,OTHERWISE SEND 0's
    int getLeftRightSamples(float *leftBuffer, float *rightBuffer, int amount);
    int getLeftFrame(void *buffer, int amount);
    int getRightFrame(void *buffer, int amount);

private:
    static constexpr  size_t samplesPerFrame = 512;
    static constexpr  size_t queueSize = 10;
    
    moodycamel::ConcurrentQueue<audioFrame> *emptyFrameQueue;
    moodycamel::ConcurrentQueue<audioFrame> *filledFrameQueue;
    

    float frameBuffers[queueSize][samplesPerFrame*2] = {};
    float leftChannelReserveBuffer[samplesPerFrame];
    float rightChannelReserveBuffer[samplesPerFrame];
    float preParseBuffer[samplesPerFrame * 2];
    

    int leftChannelReserveAmount = {};
    int leftChannelReserveIndex = {};
    int rightChannelReserveAmount = {};
    int rightChannelReserveIndex = {};

    int currentIndex = {};
    drwav wav;
    std::string filePath = {};

    size_t replayFrameDelay = {};
    size_t currFrameDelay = {};
};
#endif  