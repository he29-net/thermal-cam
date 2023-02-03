#ifndef __CONFIG_H__
#define __CONFIG_H__

// Enable or disable optional features depending on desired binary size (base size ~25.7 kB)
#define BMP_SAVE			// enable BPM image saving functionality (~4 kB)
#define CLEANUP				// flush garbage frames out of the sensor after startup and BPM saving (~100 B)

// Debug features
//#define PROFILE			// measure and display performance of the processing code (~150 B)
//#define DISPLAY_TEST		// display test pattern and stop
//#define DRAWING_TEST		// replace sensor output with a fixed test pattern

#endif
