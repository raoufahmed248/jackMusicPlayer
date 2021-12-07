#include "wavPlayer.h"

int wavPlayer::returnFour()
{
    return 4;
}

wavPlayer::wavPlayer(std::string wavFilePath, int queueSize, int sampleDelayBetweenPlays)
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
    this->queueSize = queueSize;
    replayFrameDelay = sampleDelayBetweenPlays;
    leftSampleQueue = new moodycamel::ConcurrentQueue<float[512]> (this->queueSize);
    rightSampleQueue = new moodycamel::ConcurrentQueue<float[512]> (this->queueSize);
    


}

wavPlayer::~wavPlayer()
{
    drwav_uninit(&wav);
    delete leftSampleQueue;
    delete rightSampleQueue;
}

int wavPlayer::decode()
{
    size_t numOfEmptySpaces = queueSize - leftSampleQueue->size_approx();

    size_t samplesToParse = {};
    size_t totalDecoded = {};
    if(wav.channels == 2)
    {
        samplesToParse = samplesPerFrame * 2;
    }
    else
    {
        samplesToParse = samplesPerFrame;
    }

    for(int x = 0; x < numOfEmptySpaces; x++)
    {
        if(currFrameDelay > 0)
        {
            memset(&preParseBuffer, 0 , samplesToParse * sizeof(preParseBuffer[0]));
            currFrameDelay--;
        } 
        else
        {
            size_t numOfSamplesDecoded = drwav_read_pcm_frames_f32(&wav,
                samplesToParse, (float *)preParseBuffer);
            if(numOfSamplesDecoded < (samplesToParse))
            {
                size_t remaining_samples = (samplesToParse) - numOfSamplesDecoded;
                memset(&preParseBuffer[numOfSamplesDecoded],0,remaining_samples * sizeof(preParseBuffer[0]));
                drwav_seek_to_pcm_frame(&wav, 0);
                currFrameDelay = replayFrameDelay;
            }
        }

        if(wav.channels == 2)
        {
            //Parse left channel
            for(int y = 0; y < samplesToParse; y+=2)
            {
                scratchBuffer[y] = preParseBuffer[y];
            }
            //technically we should throw exception if this fails...
            if(leftSampleQueue->try_enqueue(scratchBuffer))
            {
                totalDecoded++;
            }
            //Parse right channel
            for(int y = 1; y < samplesToParse; y+=2)
            {
                scratchBuffer[y] = preParseBuffer[y];
            }
            //technically we should throw exception if this fails...
            if(rightSampleQueue->try_enqueue(scratchBuffer))
            {
                totalDecoded++;
            }
            
        }
        else
        {
            memcpy(scratchBuffer, preParseBuffer, samplesToParse * sizeof(preParseBuffer[0]));
            if(leftSampleQueue->try_enqueue(scratchBuffer))
            {
                totalDecoded++;
            }

            if(rightSampleQueue->try_enqueue(scratchBuffer))
            {
                totalDecoded++;
            }
            
        }
    }
    return totalDecoded;
}