/*
 *  bpmShifter.h
 *  Livetronica Studio
 *
 *  Created by Aaron Leese on 2/22/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef __FL_HEADER_BPMSHIFTER__
#define __FL_HEADER_BPMSHIFTER__

#include "BPMDetect.h"
#include "FIFOSamplePipe.h"
#include "FIFOSampleBuffer.h"
#include "soundtouch_config.h"
#include "SoundTouch.h"

#include "STTypes.h"

#include "juceHeader.h"
  
class BPMShifter : public soundtouch::SoundTouch, 
	 public ValueListener
{
	
	float* transferArray; // since JUCE uses contiguous buffers, and SoundTouch uses interleaved ... we have a step of translation
	
	int processMarker;
	 
	bool processEntireBuffer;
	bool reversing;
	
public:
	
	
	AudioSampleBuffer* originalBuffer;
	AudioSampleBuffer* resampledBuffer; // only used if processEntireBuffer = true
	 
	bool soundTouchEnabled;
	
	Value* syncing;
    bool synced; // since the syncing Value is mapped to buttons ...we need to decouple it (or else setToggleState triggers click, etc. 
    
	Value* freqShiftTarget;
	Value* freqShiftBase;  // for scratchable objects, this is the exact current F shift after taking into acount turntables

	float freqShiftLast;
	float freqShiftCurrent;
	
	float lastValueAdded[2];
	
	Value* timeStretch;
	Value* pitchShift;
 
	BPMShifter();
	
	virtual ~BPMShifter();
	
	void setInputBuffer(AudioSampleBuffer* inputBuffer);
	
	void setPlayPosition(int currentOffset) {
		
		BPMShifter::clear(); // expensive ...
		processMarker = currentOffset;
	
	}
	
	void cacheOriginalAsInterleaved()
	{
		
		// Since JUCE buffers are contiguous and ST requires interleaved ... we translate it here ...
		
		//	.. this function is only needed for STEREO inputs with processEntireBuffer .... 
		// ... which is to slow .... so Im not implementing it yet ...
		
		int numChannels = originalBuffer->getNumChannels();
		int numSamples =  originalBuffer->getNumSamples();
		
		jassert(numChannels == 2);
		
		/*
		float *p  = (float*)realloc(originalInterleaved, numSamples*numChannels*sizeof(float));
		 
		if (p != NULL) 
		{
			originalInterleaved  = p; 
		}
		else 
		{
			DBG("OUT OF MEMORY");
			jassert(false);
		}
		
		
		// translate to INTERLEAVED data
		for (int chan = 0; chan < numChannels; chan++)
		{
			for (int i = 0; i < originalBuffer->getNumSamples(); i++)
				originalInterleaved[numChannels*i + chan] = *originalBuffer->getSampleData(chan, i);
		}
	
		 	*/
		 
	}
	
	 
    float getSampleProcessed(int positionInOriginal);	
		
	void valueChanged (Value &value);
 	
	void changeRate(float newRate);
	void changePitch(float newPitchShift);
	void changeTempo(float newTempoShift);
	
	// for immediate processing .....
	void processSoundTouchBlock(AudioSampleBuffer & buffer, int positionInOriginal);
	
    void processSoundTouchBlockWithGain(AudioSampleBuffer & buffer, int positionInOriginal, float gain);
        
        
	void processSoundTouchBlockWithRamp(AudioSampleBuffer & buffer, int positionInOriginal, float startGain, float endGain) {
		// the base rate shift, if there is one 
		processBaseShift(buffer, positionInOriginal);
		
		// TURNTABLE processing	
		processTurntableBlock(buffer, startGain, endGain);
		
		
	}
	
	int handleZeroCrossing(AudioSampleBuffer & buffer);
	
	// processing
	void processBaseShift(AudioSampleBuffer & buffer, int positionInOriginal);
	
	void processTurntableBlock(AudioSampleBuffer & buffer, float startGain, float endGain);
	
	void stopProcessing();	
};


/// From SoundTouch.h :
///
/// Notes:
/// - Initialize the SoundTouch object instance by setting up the sound stream 
///   parameters with functions 'setSampleRate' and 'setChannels', then set 
///   desired tempo/pitch/rate settings with the corresponding functions.
///
/// - The SoundTouch class behaves like a first-in-first-out pipeline: The 
///   samples that are to be processed are fed into one of the pipe by calling
///   function 'putSamples', while the ready processed samples can be read 
///   from the other end of the pipeline with function 'receiveSamples'.
/// 
/// - The SoundTouch processing classes require certain sized 'batches' of 
///   samples in order to process the sound. For this reason the classes buffer 
///   incoming samples until there are enough of samples available for 
///   processing, then they carry out the processing step and consequently
///   make the processed samples available for outputting.
/// 
/// - For the above reason, the processing routines introduce a certain 
///   'latency' between the input and output, so that the samples input to
///   SoundTouch may not be immediately available in the output, and neither 
///   the amount of outputtable samples may not immediately be in direct 
///   relationship with the amount of previously input samples.
///
/// - The tempo/pitch/rate control parameters can be altered during processing.
///   Please notice though that they aren't currently protected by semaphores,
///   so in multi-thread application external semaphore protection may be
///   required.
///
/// - This class utilizes classes 'TDStretch' for tempo change (without modifying
///   pitch) and 'RateTransposer' for changing the playback rate (that is, both 
///   tempo and pitch in the same ratio) of the sound. The third available control 
///   'pitch' (change pitch but maintain tempo) is produced by a combination of
///   combining the two other controls.
///


#endif