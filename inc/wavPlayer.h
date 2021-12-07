
#ifndef wavPlayer_h
#define wavPlayer_h

//#include "dr_wav.cpp"
#include "dr_wav.cpp"
#include "concurrentqueue.h"
//template<size_t frameSize>
class wavPlayer {
public:
    int returnFour();
    wavPlayer(std::string wavFilePath, int queueSize, int delayBetweenPlays);
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
    int getLeftRightSamples(void *leftBuffer, void *rightBuffer, int amount);
    int getLeftFrame(void *buffer, int amount);
    int getRightFrame(void *buffer, int amount);

private:
    static constexpr  size_t samplesPerFrame = 512;
    moodycamel::ConcurrentQueue<float[512]> *leftSampleQueue;
    moodycamel::ConcurrentQueue<float[512]> *rightSampleQueue;
    float leftChannelReserveBuffer[512] = {};
    float rightChannelReserveBuffer[512] = {};
    float preParseBuffer[512*2] = {};
    float scratchBuffer[512] = {};
    int leftChannelReserveAmount = {};
    int rightChannelReserveAmount = {};
    int currentIndex = {};
    drwav wav;
    std::string filePath = {};

    size_t replayFrameDelay = {};
    size_t currFrameDelay = {};
    size_t queueSize = {};
};
#endif  