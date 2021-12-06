#include "wavPlayer.h"
#include "dr_wav.cpp"
int wavPlayer::returnFour()
{
    return 4;
}

wavPlayer::wavPlayer(std::string wavFilePath, int queueSize, int sampleDelayBetweenPlays)
{
    drwav wav;

    if (!drwav_init_file(&wav, wavFilePath.c_str(), NULL)) {
        // Error opening WAV file.
        throw std::invalid_argument("DR_WAV COULD NOT OPEN WAV FILE!");
    }
    filePath = wavFilePath;
    leftSampleQueue = new moodycamel::ConcurrentQueue<int[512]> (queueSize);
    rightSampleQueue = new moodycamel::ConcurrentQueue<int[512]> (queueSize);
    


}

wavPlayer::~wavPlayer()
{
    //drwav_uninit(&wav);
    delete leftSampleQueue;
    delete rightSampleQueue;
}