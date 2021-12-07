#include "wavPlayer.h"

int wavPlayer::returnFour()
{
    return 4;
}

wavPlayer::wavPlayer(std::string wavFilePath, int sampleDelayBetweenPlays)
{
    if (!drwav_init_file(&wav, wavFilePath.c_str(), NULL)) {
        // Error opening WAV file.
        throw std::invalid_argument("DR_WAV COULD NOT OPEN WAV FILE!");
    }
    if(wav.channels > 2 || wav.channels < 1)
    {
        throw std::invalid_argument("THIS SOFTWARE ONLY SUPPORTS STEREO OR MONO WAV FILES (<=2 CHANNELS)!");
    }
    filePath = wavFilePath;
    
    replayFrameDelay = sampleDelayBetweenPlays;
    
    emptyFrameQueue = new moodycamel::ConcurrentQueue<audioFrame> (queueSize);
    filledFrameQueue = new moodycamel::ConcurrentQueue<audioFrame> (queueSize);
    audioFrame tempFrame = {};
    for(int x = 0; x < queueSize; x++)
    {
        tempFrame.size = samplesPerFrame;
        tempFrame.leftBuffer = &frameBuffers[x][0 * samplesPerFrame];
        tempFrame.rightBuffer = &frameBuffers[x][1 * samplesPerFrame];
        if(!emptyFrameQueue->try_enqueue(tempFrame))
        {
            throw std::invalid_argument("FAILED TO ADD INITIAL FRAMES TO EMPTY QUEUE!");
        }
    }
}

wavPlayer::~wavPlayer()
{
    drwav_uninit(&wav);
    delete emptyFrameQueue;
    delete filledFrameQueue;
}

int wavPlayer::decode()
{
    return 1;
}

int wavPlayer::getLeftRightSamples(float *leftBuffer, float *rightBuffer, int amount)
{
    return 1;

}