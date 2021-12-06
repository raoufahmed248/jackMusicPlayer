
#ifndef wavPlayer_h
#define wavPlayer_h

//#include "dr_wav.cpp"
#include "concurrentqueue.h"
//template<size_t frameSize>
class wavPlayer {
public:
    int returnFour();
    wavPlayer(std::string wavFilePath, int queueSize, int delayBetweenPlays);
    ~wavPlayer();
    int decode();
    int decode(int amount);
    int currentQueueSize();
    int currentSampleIndex();
    int totalSamples();

    void resetIndex();

    int getLeftFrame(void *buffer, int amount);
    int getRightFrame(void *buffer, int amount);

private:
    moodycamel::ConcurrentQueue<int[512]> *leftSampleQueue;
    moodycamel::ConcurrentQueue<int[512]> *rightSampleQueue;    
    int leftChannelReserveBuffer[512] = {};
    int rightChannelReserveBuffer[512] = {};
    int leftChannelReserveAmount = {};
    int rightChannelReserveAmount = {};
    int currentIndex = {};
    //drwav wav;
    std::string filePath = {};


};
#endif  