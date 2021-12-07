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
    
    leftChannelReserveBuffer.reserve(samplesPerFrame);
    rightChannelReserveBuffer.reserve(samplesPerFrame);
    preParseBuffer.reserve(samplesPerFrame * 2);
    scratchBuffer.reserve(samplesPerFrame);
    offloadingBuffer.reserve(samplesPerFrame);
    
    replayFrameDelay = sampleDelayBetweenPlays;
    leftSampleQueue = new moodycamel::ConcurrentQueue<std::vector<float>> (this->queueSize);
    rightSampleQueue = new moodycamel::ConcurrentQueue<std::vector<float>> (this->queueSize);
    


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
                samplesToParse, (float *)preParseBuffer.data());
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
            memcpy(scratchBuffer.data(), preParseBuffer.data(), samplesToParse * sizeof(preParseBuffer[0]));
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

int wavPlayer::getLeftRightSamples(float *leftBuffer, float *rightBuffer, int amount)
{
    size_t leftIndex = {};
    size_t rightIndex = {};
    
    if(leftChannelReserveAmount != rightChannelReserveAmount)
    {
        leftChannelReserveAmount = 0;
        rightChannelReserveAmount = 0;
        leftChannelReserveIndex = 0;
        rightChannelReserveIndex = 0;
    }

    size_t amountFromReserve = std::min(leftChannelReserveAmount, amount);

    memcpy(leftBuffer, &leftChannelReserveBuffer[leftChannelReserveIndex], amountFromReserve * sizeof(leftChannelReserveBuffer[0]));
    memcpy(rightBuffer, &rightChannelReserveBuffer[rightChannelReserveIndex], amountFromReserve * sizeof(rightChannelReserveBuffer[0]));
    
    leftChannelReserveAmount -= amountFromReserve;
    rightChannelReserveAmount -= amountFromReserve;
    leftChannelReserveIndex += amountFromReserve;
    rightChannelReserveIndex += amountFromReserve;
    leftIndex += amountFromReserve;
    rightIndex += amountFromReserve;

    if(leftChannelReserveAmount == 0)
    {
        leftChannelReserveIndex = 0;
        rightChannelReserveIndex = 0;
    }

    amount -= amountFromReserve;
    
    if(amount == 0)
    {
        return amountFromReserve;
    }

    size_t numOfFramesToDecode = ((amount -1) / samplesPerFrame) + 1;

    if((leftSampleQueue->size_approx() < numOfFramesToDecode) || (rightSampleQueue->size_approx() < numOfFramesToDecode))
    {
        memset(&leftBuffer[leftIndex], 0, amount * sizeof(leftBuffer[0]));
        memset(&rightBuffer[leftIndex], 0, amount * sizeof(rightBuffer[0]));
        return amount + amountFromReserve;
    }

    for(int x = 0; x < numOfFramesToDecode; x++)
    {
        leftSampleQueue->try_dequeue(offloadingBuffer);
        size_t amountToCopy = std::min(amount, (int)samplesPerFrame);
        memcpy(&leftBuffer[leftIndex], offloadingBuffer.data(), amountToCopy * sizeof(leftBuffer[0]));
        
        rightSampleQueue->try_dequeue(offloadingBuffer);
        memcpy(&rightBuffer[rightIndex], offloadingBuffer.data(), amountToCopy * sizeof(rightBuffer[0]));

        rightIndex += amountToCopy;
        leftIndex += amountToCopy;
        amount -= amountToCopy;



    }
    return leftIndex;

}