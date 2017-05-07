#ifndef __DIGITAL_DECODER_H__
#define __DIGITAL_DECODER_H__

#include <stdint.h>
#include <map>

class DigitalDecoder
{
  public:
    DigitalDecoder() = default;
    
    void handleData(char data);
    
  private:
  
    void writeDeviceState();
    void sendDeviceState();
    void updateDeviceState(uint32_t serial, uint8_t state);
    void handlePayload(uint64_t payload);
    void handleBit(bool value);
    void decodeBit(bool value);
    void dumpStateMap(uint32_t serial);

    unsigned int samplesSinceEdge = 0;
    bool lastSample = false;
    
    struct deviceState_t
    {
        uint64_t lastUpdateTime;
        uint8_t lastRawState;
       
	bool loop1;
	bool loop2;
	bool loop3;
	bool loop4; 

	bool supervision;
        bool batteryLow;
    };

    std::map<uint32_t, deviceState_t> deviceStateMap;
};

#endif
