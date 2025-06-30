//Per https://arduino-craft-corner.de/index.php/2023/11/29/replacing-the-wire-library-sometimes/
//this is the way that we use arduino library dependancy to replace the wire library on a per-sketch basis (e.g. without modifying underlying 3rd party libraries)

#ifndef Wire_h
#define Wire_h

#include <FlexWire.h> //Drop-in library that overrides the wire lib to use bit-banging on any pins.
//The example says to use "SoftwareWire", but the blog post was made before FlexWire was released.  Replace every SoftWire with FlexWire, and it works just great.


extern FlexWire Wire;
typedef FlexWire TwoWire;

#endif 
