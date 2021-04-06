#include "digitalDecoder.h"
#include "config.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>


#define SYNC_MASK    0xFFFF000000000000ul
#define SYNC_PATTERN 0xFFFE000000000000ul


void DigitalDecoder::writeAllDeviceState()
{
    std::ofstream file;
    file.open(C_STATE_FILE);
    
    if(!file.is_open())
    {
        printf("Could not open JSON file\n");
        return;
    }
    
    bool isFirst = true;
    
    file << "[" << std::endl;
    for(const auto &dd : deviceStateMap)
    {
        if(!isFirst) file << "," << std::endl;
        isFirst = false;
        
        file << "{" << std::endl;
        file << "    \"serial\": " << dd.first << "," << std::endl;
        file << "    \"loop1\": " << (dd.second.loop1 ? "true," : "false,") << std::endl;
        file << "    \"loop2\": " << (dd.second.loop2 ? "true," : "false,") << std::endl;
        file << "    \"loop3\": " << (dd.second.loop3 ? "true," : "false,") << std::endl;
        file << "    \"loop4\": " << (dd.second.loop4 ? "true," : "false,") << std::endl;
        file << "    \"batteryLow\": " << (dd.second.batteryLow ? "true," : "false,") << std::endl;
        file << "    \"supervision\": " << (dd.second.supervision ? "true," : "false,") << std::endl;
        file << "    \"lastUpdateTime\": " << dd.second.lastUpdateTime << std::endl;
        //file << "    \"lastUpdateTime\": " << dd.second.lastUpdateTime << "," << std::endl;
        file << "}";
    }
    
    file << std::endl << "]" << std::endl;
    
    file.close();
}

//#warning "Update the SmartThings endpoint here"
void DigitalDecoder::sendDeviceState()
{

#ifdef DEBUG
    printf("\n*** Sending Device State ***\n");
#endif

    if (C_UPDATE_CMD != "")
    {
        system(C_UPDATE_CMD);
    }
}

void DigitalDecoder::printDeviceState(uint32_t serial)
{
    deviceState_t ds;

    if(deviceStateMap.count(serial))
    {
        ds = deviceStateMap[serial];
    }
    else
    {
        return;
    }

    std::cout << "{";
    std::cout << "\"serial\": " << serial << ",";
    std::cout << "\"loop1\": " << (ds.loop1 ? "true," : "false,");
    std::cout << "\"loop2\": " << (ds.loop2 ? "true," : "false,");
    std::cout << "\"loop3\": " << (ds.loop3 ? "true," : "false,");
    std::cout << "\"loop4\": " << (ds.loop4 ? "true," : "false,");
    std::cout << "\"batteryLow\": " << (ds.batteryLow ? "true," : "false,");
    std::cout << "\"supervision\": " << (ds.supervision ? "true," : "false,");
    std::cout << "\"lastUpdateTime\": " << ds.lastUpdateTime;
    std::cout << "}" << std::endl;
}

void DigitalDecoder::updateDeviceState(uint32_t serial, uint8_t state)
{
    deviceState_t ds;
    bool forceUpdate = false;

    // Extract prior info
    if(deviceStateMap.count(serial))
    {
        ds = deviceStateMap[serial];
    }

    // Decode bits
    ds.loop1 = (state & 0x80);
    ds.loop2 = (state & 0x20);
    ds.loop3 = (state & 0x10);
    ds.loop4 = (state & 0x40);
    ds.batteryLow = (state & 0x02);
    ds.supervision = (state & 0x04);

    // Timestamp
    timeval now;
    gettimeofday(&now, nullptr);

    // Some devices, such as keyfobs, do not send a reset, so it
    // is best to consider elapsed time as well as state change
    if ((now.tv_sec - ds.lastUpdateTime) > C_REPEAT_TIME)
    {
        forceUpdate = true;
#ifdef DEBUG
	printf("repeat time elapsed, forcing update\n");
#endif
    }

    //std::cout << "lastUpdateTime " << ds.lastUpdateTime << std::endl;
    //std::cout << "now            " << now.tv_sec << std::endl;

    // Update
    ds.lastUpdateTime = now.tv_sec;
    
    // Put the answer back in the map
    deviceStateMap[serial] = ds;
    
    // Record the current state
    writeAllDeviceState();
    
    // Send the notification if something changed
    if(state != ds.lastRawState || forceUpdate == true)
    {
        sendDeviceState();
        printDeviceState(serial);
#ifdef SHOW
        dumpStateMap(serial);
#endif
    }

    // Update raw state
    deviceStateMap[serial].lastRawState = state;

#ifdef DEBUG
    dumpStateMap(serial);
#endif

}

void DigitalDecoder::dumpStateMap(uint32_t serial)
{
    time_t rawtime;
    struct tm * timeinfo;
    char s[128];
    int count = 0;

    // Obtain and output current date and time
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(s,sizeof(s),C_DATETIME_FORMAT,timeinfo);
    printf("%s\n",s);
 
    for(const auto &dd : deviceStateMap)
    {
        count++;

        printf("%sDevice %07u: %s%s%s%s%s%s\n",
	       dd.first==serial ? "*" : " ", dd.first,
               dd.second.loop1 ? "L1 " : "",
               dd.second.loop2 ? "L2 " : "",
               dd.second.loop3 ? "L3 " : "",
               dd.second.loop4 ? "L4 " : "",
               dd.second.supervision ? "SUP " : "",
               dd.second.batteryLow ? "BATT" : ""
	); 
    }
    printf("Total devices: %d\n",count);
    printf("\n");
    fflush(stdout);

}

void DigitalDecoder::handlePayload(uint64_t payload)
{
    uint64_t sof = (payload & 0xF00000000000) >> 44;
    uint64_t ser = (payload & 0x0FFFFF000000) >> 24;
    uint64_t typ = (payload & 0x000000FF0000) >> 16;
    uint64_t crc = (payload & 0x00000000FFFF) >>  0;
    
    //
    // Check CRC
    //
    const uint64_t polynomial = 0x18005;
    uint64_t sum = payload & (~SYNC_MASK);
    uint64_t current_divisor = polynomial << 31;
    
    while(current_divisor >= polynomial)
    {
#ifdef __arm__
        if(__builtin_clzll(sum) == __builtin_clzll(current_divisor))
#else        
        if(__builtin_clzl(sum) == __builtin_clzl(current_divisor))
#endif
        {
            sum ^= current_divisor;
        }        
        current_divisor >>= 1;
    }
    
    const bool valid = (sum == 0);
    
    //
    // Tell the world
    //
    if(valid)
    {
        updateDeviceState(ser, typ);
    }
    
    
    //
    // Print Packet
    //

#ifdef DEBUG
#ifdef __arm__
    if(valid)    
        printf("Valid Payload: %llX (Serial %llu, Status %llX)\n", payload, ser, typ);
    else
        printf("Invalid Payload: %llX\n", payload);
#else    
    if(valid)    
        printf("Valid Payload: %lX (Serial %lu, Status %lX)\n", payload, ser, typ);
    else
        printf("Invalid Payload: %lX\n", payload);

    fflush(stdout);
#endif
#endif   
 
    static uint32_t packetCount = 0;
    static uint32_t errorCount = 0;
    
    packetCount++;
    if(!valid)
    {
        errorCount++;
#ifdef DEBUG
        printf("%u/%u packets failed CRC\n", errorCount, packetCount);
#endif
    }
}



void DigitalDecoder::handleBit(bool value)
{
    static uint64_t payload = 0;
    
    payload <<= 1;
    payload |= (value ? 1 : 0);
    
//#ifdef __arm__
//    printf("Got bit: %d, payload is now %llX\n", value?1:0, payload);
//#else
//    printf("Got bit: %d, payload is now %lX\n", value?1:0, payload);
//#endif     
    
    if((payload & SYNC_MASK) == SYNC_PATTERN)
    {
        handlePayload(payload);
        payload = 0;
    }
}

void DigitalDecoder::decodeBit(bool value)
{
    enum ManchesterState
    {
        LOW_PHASE_A,
        LOW_PHASE_B,
        HIGH_PHASE_A,
        HIGH_PHASE_B
    };
    
    static ManchesterState state = LOW_PHASE_A;
    
    switch(state)
    {
        case LOW_PHASE_A:
        {
            state = value ? HIGH_PHASE_B : LOW_PHASE_A;
            break;
        }
        case LOW_PHASE_B:
        {
            handleBit(false);
            state = value ? HIGH_PHASE_A : LOW_PHASE_A;
            break;
        }
        case HIGH_PHASE_A:
        {
            state = value ? HIGH_PHASE_A : LOW_PHASE_B;
            break;
        }
        case HIGH_PHASE_B:
        {
            handleBit(true);
            state = value ? HIGH_PHASE_A : LOW_PHASE_A;
            break;
        }
    }
}

void DigitalDecoder::handleData(char data)
{
    static const int samplesPerBit = 8;
    
        
    if(data != 0 && data != 1) return;
        
    const bool thisSample = (data == 1);
    
    if(thisSample == lastSample)
    {
        samplesSinceEdge++;
        
        //if(samplesSinceEdge < 100)
        //{
        //    printf("At %d for %u\n", thisSample?1:0, samplesSinceEdge);
        //}
        
        if((samplesSinceEdge % samplesPerBit) == (samplesPerBit/2))
        {
            // This Sample is a new bit
            decodeBit(thisSample);
        }
    }
    else
    {
        samplesSinceEdge = 1;
    }
    lastSample = thisSample;
}
