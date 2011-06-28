/*
 *  bpmShifter.cpp
 *  Livetronica Studio
 *
 *  Created by Aaron Leese on 2/22/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "bpmShifter.h" 
 
BPMShifter::BPMShifter() : soundtouch::SoundTouch() {	
	
	soundTouchEnabled = true;
	
	processEntireBuffer = false;
	
	freqShiftTarget = new Value(1); // is trying to get to this ....
	freqShiftBase = new Value(1);
	timeStretch = new Value(1);
	pitchShift = new Value(1);
	
	freqShiftTarget->addListener(this);
	freqShiftBase->addListener(this);
	timeStretch->addListener(this);
	pitchShift->addListener(this);
	
	syncing = new Value(true);
	
	setRate(1);
	setPitch(1);
	setTempo(1);
	
	lastValueAdded[0] = lastValueAdded[1] = 0;
	
	reversing = false;
	
	resampledBuffer = new AudioSampleBuffer(1, 500000);
	
	setSampleRate(44100);
	//setChannels(uint(resampledBuffer->getNumChannels())); done when the input buffer is set ...
	
	int AA = getSetting(1); // AA Filter length .... check it out ...
	//setSetting(0, false); // turn off the AA

	//DBG("setup soundtouch  " +  String(myAudioManager->deviceSetup.sampleRate));
	
	originalBuffer = 0;
	processMarker = 0; 
	 
	transferArray = (float*)calloc(350000, sizeof(float));
	freqShiftCurrent = freqShiftLast = 1;
	
}

BPMShifter::~BPMShifter() {
	
	DBG("~BPMShifter()");
	 
	if (resampledBuffer != 0) delete resampledBuffer; 

	freqShiftTarget->removeListener(this);
	freqShiftBase->removeListener(this);
	timeStretch->removeListener(this);
	pitchShift->removeListener(this);
	syncing->removeListener(this);

	
    if (freqShiftTarget != 0) delete freqShiftTarget;
	if (freqShiftBase != 0) delete freqShiftBase;
	if (timeStretch != 0) delete timeStretch;
	if (pitchShift != 0) delete pitchShift;
	if (syncing != 0) delete syncing;
	 
	
}
 
void BPMShifter::setInputBuffer(AudioSampleBuffer* inputBuffer) {
	
	processMarker = 0;
	
	originalBuffer = inputBuffer;

	DBG("soundtouch setInput channels: " + String(originalBuffer->getNumChannels()))
	setChannels(inputBuffer->getNumChannels());			
	
    DBG("clearing soudtouch ....");
    setChannels(originalBuffer->getNumChannels());			
    SoundTouch::clear(); // in case there is some crap in there currently ...
	
	 
	
}

// STREAMING ...
void BPMShifter::processSoundTouchBlock(AudioSampleBuffer & buffer, int positionInOriginal) {
	
	jassert(buffer.getNumChannels() == 2);
	
	// the base rate shift, if there is one 
	processBaseShift(buffer, positionInOriginal);
		 
	processTurntableBlock(buffer, 1, 1);
	
}

void BPMShifter::processSoundTouchBlockWithGain(AudioSampleBuffer & buffer, int positionInOriginal, float gain) {
    
    jassert(buffer.getNumChannels() == 2);
	
	// the base rate shift, if there is one 
	processBaseShift(buffer, positionInOriginal);
    
	processTurntableBlock(buffer, gain, gain);
    
}

void BPMShifter::processBaseShift(AudioSampleBuffer & buffer, int positionInOriginal) {
	
	//
	//	PROCESS THE SOUNDTOUCH OBJECT STREAM
	//
	//  feed samples to the SoundTouch object until we have enough
	//	returned samples to satisfy the turntables.
	//  
	//
	///////////////////////////////////////////////////////
	
	if (originalBuffer == 0) return;
	
	jassert(buffer.getNumChannels() == 2);
	int numSamples = buffer.getNumSamples();
	int numChannels = originalBuffer->getNumChannels();
	
	int numNeeded = numSamples; // the number needed for the TT process step ...  
	int blockSize = 64; // the size blocks that we will feed soundTouch .. not sure whats optimum, but 64 seems pretty good, performance wise ....
	if (freqShiftCurrent != 1 || freqShiftLast != 1)
        numNeeded = numSamples*(fabsf(freqShiftLast) + fabsf(freqShiftCurrent))/2; 
	
	// ZERO CROSSING :::::
	int numberProcessedPreZeroCrossing = 0;
	if ((freqShiftCurrent > 0 && freqShiftLast < 0) || (freqShiftCurrent < 0 && freqShiftLast > 0) )
	{
		
        // GET a BLOCK of data prior to the zero crossing ....
		numberProcessedPreZeroCrossing = handleZeroCrossing(buffer);
			
		// reset ... 
		BPMShifter::clear();
		
        // now CHANGE variables so we can grab the second part of the data (AFTER the zero crossing)
        // tricky here ... if WAS moving forward ... then new processing position is Position, PLUS num done already, plus AA size ...
		if (freqShiftLast > 0) processMarker = positionInOriginal + numberProcessedPreZeroCrossing + 32; // 32 is the size of the AA filter buffer
		else processMarker = positionInOriginal - numberProcessedPreZeroCrossing - 32;
		
        // rollover check .... just in case ...
        if (processMarker > originalBuffer->getNumSamples() || processMarker < 0)
            processMarker = (processMarker + originalBuffer->getNumSamples())%originalBuffer->getNumSamples();
        
		jassert(numNeeded >= numberProcessedPreZeroCrossing);
		numNeeded -= numberProcessedPreZeroCrossing;
        
        
	}
	
	int polarity = 1; // FORWARD
	if (freqShiftCurrent < 0) polarity = -1; // REVERSE
	 		
	// ADD samples until we have enough ...
	while (BPMShifter::numSamples() <= numNeeded) // we always keep 10 times what we need immediately available
	{
        // translate to INTERLEAVED data
		for (int chan = 0; chan < numChannels; chan++)
		{
			for (int i = 0; i < blockSize; i++)
				transferArray[numChannels*(i+numberProcessedPreZeroCrossing) + chan] = *originalBuffer->getSampleData(chan, int(processMarker + polarity*i + originalBuffer->getNumSamples())%originalBuffer->getNumSamples());
		}
		
        
		putSamples(&transferArray[2*numberProcessedPreZeroCrossing], blockSize);
		processMarker = (processMarker + polarity*blockSize + originalBuffer->getNumSamples())%originalBuffer->getNumSamples();
		
	}
  	
	// GET the data back from SoundTouch 
	int numRecieved = receiveSamples(&transferArray[2*numberProcessedPreZeroCrossing], numNeeded);
	int underrunAmount = numNeeded - numRecieved;
	if (underrunAmount > 0 && freqShiftCurrent != 0) 
	{
		DBG("underrun " + String(underrunAmount));
		jassert(false);
	}
	
	// SPECIAL CASE :::: we must get the last value added last time and replicate it for the TTs to use
	if (freqShiftCurrent == 0 && freqShiftLast == 0) 
	{
		transferArray[0] = lastValueAdded[0];
		transferArray[1] = lastValueAdded[1];
		
	}

	
	
}
 
int BPMShifter::handleZeroCrossing(AudioSampleBuffer & buffer) {
	
	//
	//
	// ::::::::: HEAVY MAGIC HERE ::::::
	//
	//  Since we may go from + to - freqShift within a block, it is important to add
	//  the right number of samples in each direction to the input buffer for SoundTouch
	//  This way, Freq will hit zero and then pick up in the opposite direction within a single buffer .... wahoooo!!!!!
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	float numSamples = buffer.getNumSamples();
	int numChannels = originalBuffer->getNumChannels();

	int numNeeded = float(numSamples)*(fabsf(freqShiftCurrent + freqShiftLast))/2.0f;  
    
    float numberPreCrossing = 0;
    if ((freqShiftCurrent > 0 && freqShiftLast < 0) || (freqShiftCurrent < 0 && freqShiftLast > 0) )
        numberPreCrossing = fabsf(freqShiftLast) / (fabsf(freqShiftCurrent) + fabsf(freqShiftLast)) * float(numSamples);
    
    
    // number PreCrossing
    //bufferPosShifted = numNeeded*(float(bufferPos)/float(numSamples));
    int numberRequiredByTurntable = numNeeded*(float(numberPreCrossing)/float(numSamples));

    //	int numberNeededPreZeroCrossing = fabsf(freqShiftLast) / (fabsf(freqShiftCurrent) + fabsf(freqShiftLast)) * float(numSamples);
	
    // number requires BY TT to ensure OUT gets the numberNeededPreZeroCrossing ...
	//int numberRequiredByTurntable = numberNeededPreZeroCrossing*(numNeeded/numSamples);
	// bufferPosShifted = numNeeded*(float(bufferPos)/float(numSamples));
    
    //int numberRequiredByTurntable = numNeeded*(float(numberNeededPreZeroCrossing)/float(numSamples));
    
    DBG("HANDLE 0 CROSSING at " + String(numberPreCrossing) + " -> " + String(numberRequiredByTurntable) + "/" + String(numNeeded));
	
	
	int blockSize = 400;
	
	// ADD samples until we have enough ...
	while (BPMShifter::numSamples() <= numberRequiredByTurntable) // we always keep 10 times what we need immediately available
	{
		if (freqShiftLast > 0) // FORWARD
		{
			
			// translate to INTERLEAVED data
			for (int chan = 0; chan < numChannels; chan++)
			{
				for (int i = 0; i < blockSize; i++)
					transferArray[numChannels*(i) + chan] = *originalBuffer->getSampleData(chan, int(processMarker + i)%originalBuffer->getNumSamples());
			}
			
			putSamples(transferArray, blockSize);
			processMarker = (processMarker + blockSize)%originalBuffer->getNumSamples();
		}
		else // REVERSING 
		{
			for (short chan = 0; chan < numChannels; chan++)
			{
				for (short i = 0; i < blockSize; i++)
					transferArray[numChannels*i + chan] =  *originalBuffer->getSampleData(chan, int(processMarker - i  + originalBuffer->getNumSamples())%originalBuffer->getNumSamples());
			}
			
			putSamples(transferArray, blockSize);
			processMarker = (processMarker - blockSize + originalBuffer->getNumSamples())%originalBuffer->getNumSamples();
		} 

	}
	
	// GET the data back from SoundTouch 
	receiveSamples(transferArray, numberRequiredByTurntable);
	
	// Now ... since we will need to buffer further, return the amount added ....
	return numberRequiredByTurntable;
}

void BPMShifter::processTurntableBlock(AudioSampleBuffer & buffer, float startGain, float endGain) {
	
	int numSamples = buffer.getNumSamples();
	int numNeeded = float(numSamples)*(fabsf(freqShiftCurrent + freqShiftLast))/2.0f;  
    
    float numberPreCrossing = 0;
    if ((freqShiftCurrent > 0 && freqShiftLast < 0) || (freqShiftCurrent < 0 && freqShiftLast > 0) )
        numberPreCrossing = fabsf(freqShiftLast) / (fabsf(freqShiftCurrent) + fabsf(freqShiftLast)) * float(numSamples);
    
         
	// NORMAL PLAY ::: optimization
	if (freqShiftLast == 1 && freqShiftCurrent == 1)
	{
		for (short chan=0; chan < buffer.getNumChannels(); chan++)
		{
			for (short bufferPos = 0; bufferPos < numSamples; bufferPos++)
				*buffer.getSampleData(chan, bufferPos) += startGain*transferArray[2*int(bufferPos) + chan];
		}
		
		return;
		
	}
	
    
    return;
   	
    // GET THE SAMPLE(S) ... mix em down
	for (short chan=0; chan < buffer.getNumChannels(); chan++)
	{
		float bufferPosShifted = 0;
	 	
		/// BUFFER COPY ::::::::::::::::::::
		for (short bufferPos = 0; bufferPos < numSamples; bufferPos++)
		{
			
			// SMOOTH THE FREQ SHIFTING
          
            // linear interpolation ...
            bufferPosShifted = numNeeded*(float(bufferPos)/float(numSamples));
			 
			*buffer.getSampleData(chan, bufferPos) = transferArray[2*int(bufferPosShifted) + chan];
			
			jassert(bufferPosShifted < numNeeded || numNeeded == 0);
			
            /*
			// SMOOTHING for SLOOOOOOW movement
			if (freqShift_smoothed < .5)
			{
				float next = *buffer.getSampleData(chan, bufferPos); // default to current
				
				// smooth this sample and next ..
			
             if (freqShift_smoothed > 0 && bufferPosShifted + 1 < numNeeded ) 
					next = transferArray[2*int(bufferPosShifted+1) + chan];					
				else if (bufferPosShifted >= 1)  next = transferArray[2*int(bufferPosShifted-1) + chan];
				
				double temp = 0;
				float fractional = modf(bufferPosShifted, &temp); // get the fractional portion of the bufferPosition
				
             
                
				*buffer.getSampleData(chan, bufferPos) = (*buffer.getSampleData(chan, bufferPos))*(1-fractional) + next*fractional;
				
				// attenuate very slooooww signals ...
				// needed because when we shift polarity, soundtouch drops gain to 0 as it fills the AA filter
				if (fabsf(freqShift_smoothed) < .1)  *buffer.getSampleData(chan, bufferPos) *= 10*fabsf(freqShift_smoothed);
				
			}
             */
            
            // ATTENUATE very slooooww signals ...
            // NEEDED ! because when we shift polarity, soundtouch drops gain to 0 as it fills the AA filter
            float freqShift_current = freqShiftLast + (freqShiftCurrent - freqShiftLast)*float(bufferPos)/float(numSamples);
            
           /* if (bufferPos == int(numberPreCrossing) && numberPreCrossing != 0 )
            {              
                DBG("cross : " + String(bufferPos) + "->" + String(bufferPosShifted) + " " + String(freqShift_current));
                *buffer.getSampleData(chan, bufferPos) = 1;
            }
            
              */
            
            
            // some FADES ::: NEEDED ! because soundtouch will fade us in from 0!
            if (fabsf(freqShift_current) < 0.001)  
            {
                *buffer.getSampleData(chan, bufferPos) = 0;   
            }
            else if (fabsf(freqShift_current) < 0.1)  
            { 
                double mult = 10*fabs(freqShift_current);
                
                *buffer.getSampleData(chan, bufferPos) = (*buffer.getSampleData(chan, bufferPos))*mult;
            }
          
             
		}
		
		
	}  
	
	// stored values for when we do shifts of 0 ....
	lastValueAdded[0] = *buffer.getSampleData(0, numSamples-1);
	lastValueAdded[1] = *buffer.getSampleData(1, numSamples-1);
	
}

// NOT STREAMING ...
 
float BPMShifter::getSampleProcessed(int positionInOriginal) {
	
	jassert(resampledBuffer->getNumChannels() == 1);
	
	if (positionInOriginal < 0) return 0;
	if (originalBuffer == 0) return 0;
	
	// tempo and rate are the two soundTouch variables that tell you the effective speed ...
	int newPosition = tempo*rate*positionInOriginal;
	
	if (newPosition >= tempo*rate*originalBuffer->getNumSamples()) return 0;
	
	if (newPosition >= resampledBuffer->getNumSamples()) return 0;
	
	return *resampledBuffer->getSampleData(0, newPosition);
	
}
 
void BPMShifter::valueChanged (Value &value) { 
	 
    
	if (value.refersToSameSourceAs(*freqShiftBase))
	{
		changeRate(freqShiftBase->getValue());
	}
	else if (value.refersToSameSourceAs(*timeStretch))
	{
		changeTempo((float)timeStretch->getValue());
	}
	else if (value.refersToSameSourceAs(*pitchShift))
	{
		changePitch(pitchShift->getValue());
	}
	 
	
}

void BPMShifter::changeRate(float newRate) {
	
	jassert(newRate > 0);
	
	DBG("changeRate() " + String(newRate));
	
	// No audio
	//if (myAudioManager->getCurrentProcessor() == 0) return;
	
    if (newRate == rate) return;
	
	// pulled ...
	if (newRate > 0) setRate(newRate);
	else setRate(-newRate);
	
	
	synced = false;
    
}

void BPMShifter::changePitch(float newPitchShift) {
	
	
	if (originalBuffer == 0) return;
	
	if (newPitchShift == 0) return;
    
    if (newPitchShift == rate) return;
	
    DBG("changePitch() " + String(newPitchShift) + " old " + String(rate));
	
	setPitch(newPitchShift);
	
	// setPitchSemiTones(float) also exists ...
	
	 
}

void BPMShifter::changeTempo(float newTempoShift) {
	
	
	if (newTempoShift == 0) return;
	
	if (originalBuffer == 0) return;
	
    if (newTempoShift == tempo) return;
    
	setTempo(newTempoShift);
	
	synced = false;
}


