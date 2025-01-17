#include "definitions_cxx.hpp"
#include "io/midi/midi_device_manager.h"

namespace HIDSysex {
void requestOLEDDisplay(MIDIDevice* device, uint8_t* data, int len);
void request7SegDisplay(MIDIDevice* device, uint8_t* data, int len);
void sysexReceived(MIDIDevice* device, uint8_t* data, int len);
void sendOLEDData(MIDIDevice* device, bool rle);

} // namespace HIDSysex
