#pragma once

#include <JuceHeader.h>

// Raw audio fifo for converting frame size
class FIFO
{
public:
    std::unique_ptr<juce::AbstractFifo> lockFreeFifo;
    juce::Array<float> data;
    int lastReadPos = 0;

    int capacity =  10240; // 512 * 20
    int low = 1024; // 512 * 2
    int high =  9261; // 512 * 18

    FIFO(){
        lockFreeFifo = std::make_unique<juce::AbstractFifo>(capacity);
        data.ensureStorageAllocated(capacity);
        juce::FloatVectorOperations::clear(data.getRawDataPointer(), capacity);

        while (data.size() < capacity){
            data.add(0.f);
        }
    }

    void setFrameSize(int newSize){
        lockFreeFifo->setTotalSize(newSize * 20);
        data.ensureStorageAllocated(newSize * 20);
        juce::FloatVectorOperations::clear(data.getRawDataPointer(), newSize * 20);

        while (data.size() < newSize * 20){
            data.add(0.f);
        }
        capacity =  newSize * 20;
        low = newSize * 2;
        high =  newSize * 18;
    }

    void writeTo(const float* writeData, int numToWrite)
    {
        int start1, start2, blockSize1, blockSize2;
        lockFreeFifo->prepareToWrite(numToWrite, start1, blockSize1, start2, blockSize2);
        if (blockSize1 > 0)
            juce::FloatVectorOperations::copy(data.getRawDataPointer() + start1, writeData, blockSize1);
        if (blockSize2 > 0)
            juce::FloatVectorOperations::copy(data.getRawDataPointer() + start2, writeData + blockSize1, blockSize2);
        lockFreeFifo->finishedWrite(blockSize1 + blockSize2);
    }

    void readFrom(float* readData, int numToRead)
    {
        int start1, start2, blockSize1, blockSize2;
        lockFreeFifo->prepareToRead(numToRead, start1, blockSize1, start2, blockSize2);
        if (blockSize1 > 0){
            juce::FloatVectorOperations::copy(readData, data.getRawDataPointer() + start1, blockSize1);
            lastReadPos = start1 + blockSize1;
        }
        if (blockSize2 > 0){
            juce::FloatVectorOperations::copy(readData + blockSize1, data.getRawDataPointer() + start2, blockSize2);
            lastReadPos = start2 + blockSize2;
        }

        lockFreeFifo->finishedRead(blockSize1 + blockSize2);
    }

    // void readMostRecent(float* readData, int numToRead){

    // }

    int writeToArray(juce::Array<float>* dest, int destPos)
    {
        while (getNumReady() > dest->size()){
            lockFreeFifo->finishedRead(getNumReady() - dest->size());
        }

        const int numToAppend = getNumReady();

        if (destPos + numToAppend < dest->size()){
            readFrom(&dest->getRawDataPointer()[destPos], numToAppend);
        }
        else{
            int toTheEnd = dest->size() - destPos;
            readFrom(&dest->getRawDataPointer()[destPos], toTheEnd);
            readFrom(&dest->getRawDataPointer()[0], numToAppend - toTheEnd);
        }
        return numToAppend;
    }

    int getNumReady()
    {
        return lockFreeFifo->getNumReady();
    }
};