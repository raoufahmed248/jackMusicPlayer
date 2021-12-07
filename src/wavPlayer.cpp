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
    size_t numFramesToFill = emptyFrameQueue->size_approx();

    size_t samplesToRetrieve = samplesPerFrame;

    if(wav.channels == 2)
    {
        samplesToRetrieve *= 2;
    }

    for(int x = 0; x < numFramesToFill; x++)
    {
        audioFrame poppedFrame = {};
        if(!emptyFrameQueue->try_dequeue(poppedFrame))
        {
            throw std::invalid_argument("FAILED TO POP EMPTY FRAMES!");
        }

        //Fill up the preParsebuffer
        if(currFrameDelay > 0)
        {
            memset(preParseBuffer, 0, samplesToRetrieve * sizeof(preParseBuffer[0]));
            
            currFrameDelay--;
        }
        else
        {
            size_t numOfSamplesDecoded = drwav_read_pcm_frames_f32(&wav,
                samplesToRetrieve, preParseBuffer);
            
            if(numOfSamplesDecoded%2 != 0 && wav.channels == 2)
            {
                throw std::invalid_argument("NUM OF SAMPLES DECODED WAS NOT MULTIPLE OF TWO!");
            }

            if(numOfSamplesDecoded < samplesToRetrieve)
            {
                size_t remainingSamples = samplesToRetrieve - numOfSamplesDecoded;
                memset(&preParseBuffer[numOfSamplesDecoded], 0, remainingSamples * sizeof(preParseBuffer[0]));
                drwav_seek_to_pcm_frame(&wav, 0);
                currFrameDelay = replayFrameDelay;
            }
        }

        //Transfer data from preParseBuffer to audioFrame
        if(wav.channels == 2)
        {
            //Parse left channel
            int audioFrameIndex = 0;
            for(int y = 0; y < samplesToRetrieve;y+=2)
            {
                poppedFrame.leftBuffer[audioFrameIndex] = preParseBuffer[y];
                poppedFrame.rightBuffer[audioFrameIndex] = preParseBuffer[y+1];
                audioFrameIndex++;
            }
            poppedFrame.size = samplesPerFrame;
        }
        else
        {
            for(int y = 0; y < samplesToRetrieve; y++)
            {
                poppedFrame.leftBuffer[y] = preParseBuffer[y];
                poppedFrame.rightBuffer[y] = preParseBuffer[y];
                
            }
        }
        if(!filledFrameQueue->try_enqueue(poppedFrame))
        {
            throw std::invalid_argument("FAILED TO PUSH FULL FRAME!");
        }

    }
    
    return numFramesToFill;

}

int wavPlayer::getLeftRightSamples(float *leftBuffer, float *rightBuffer, int amount)
{
    return 1;

}