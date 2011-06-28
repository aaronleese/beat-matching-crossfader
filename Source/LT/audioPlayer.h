/*
 *  AudioPlayer.h
 *  Livetronica Studio
 *
 *  Created by Aaron Leese on 4/4/10.
 *  Copyright 2010 StageCraft Software. All rights reserved.
 *
 */

#ifndef __JUCER_HEADER_AudioPlayer_
#define __JUCER_HEADER_AudioPlayer_

#include "juceHeader.h" 
#include "bpmShifter.h"


#include "JsonToJuce.h"

//============================================================================== The AudioPlayer player
class AudioPlayer : public Timer,
	public ChangeListener,
	public DeletedAtShutdown,
    public Thread,
	public BPMShifter
{
	//
	// Audio AudioPlayer Player .. capable of scratching
	//
	// this will play an audio file and intellegently refresh the portion of the cicular buffer furthest away from the play marker ...
	//
	/////////////////////////////////////////////////////////////////////////////////
	 
public:
	 	
	//  the reader for the file
	AudioFormatReader* reader;
	String trackName;
	int64 readerPos;
	
	Image* thumbImage;
	Graphics* thumbGraphic;
	
	AudioThumbnail* audioThumb;
	AudioThumbnailCache* audioThumbCache;
	
    CriticalSection audioPlayerLock;
    
	// for loading AudioPlayers ... caching
	AudioSampleBuffer* trackBuffer;
	int playPos;
	
	// the immediate buffer .... processed and ready to play ....
	AudioSampleBuffer* bufferProcessed; 
	float freq_conversion;
	bool playing;
	bool beginSyncedPlay;
	
    float volume;
    float bpm;
    
    int currentBeatIndex;
	
    ValueTree* myTrackInformation;
    
    JsonToJuceReader* myReader;
    
	AudioPlayer();
	
	~AudioPlayer();
	
	void processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
	
	void calculateCurrentFreq();

    int getSamplesToNextBeat();

		
	void init(AudioDeviceManager* deviceManager);
	void shutdown();
	void timerCallback();
	
    void setVolume(float newVolume)
    {
        volume = newVolume;
    }
    
	void loadFile(const File &fileToLoad);
     
    ValueTree analyzeTrack(ValueTree & trackInLib);
    
    // get UI
	Component* getEditor(); 
	Component* getTTEditor();
	 
	void changeListenerCallback (ChangeBroadcaster* objectThatHasChanged)
	{
		// called when the audioThumb has changed .... we would trigger a repaint, but for now it's on a timer
		
	}
	
	void run(); 
};

//============================================================================== the UI
class AudioPlayerUI  : public Component,
    public ButtonListener, 
    public DragAndDropTarget
{
public:
	
	DrawableButton* play;
	DrawableButton* stop;
	DrawableButton* load;
	
	AudioPlayer* myAudioPlayer;
	
	//============================================================================== AudioPlayer Player
	AudioPlayerUI(AudioPlayer* _myAudioPlayer)
	{
		
		DBG (String("new audio player UI"));
		myAudioPlayer = _myAudioPlayer;
		
		Path myPath;
		DrawablePath playTriangle;
		DrawablePath playActive;
		
		myPath.addTriangle(2,0,2,40,37,20);
		
		playTriangle.setStrokeThickness(8);
		playTriangle.setStrokeFill(FillType(Colours::green));
		playTriangle.setPath(myPath);
		playTriangle.setStrokeThickness(8);
		playTriangle.setStrokeFill(FillType(Colour(0xff22ff22)));
		playActive.setPath(myPath);
		
		addAndMakeVisible (play = new DrawableButton (T("play"), juce::DrawableButton::ImageOnButtonBackground));
		play->setButtonText (T("Play"));
		play->addListener (this);
		play->setBackgroundColours(Colour(0x77333333), Colour (0xaaaa0000));
		play->setImages(&playTriangle, 0, 0, 0, &playActive, 0, 0, 0);
		
		DrawablePath stopPath;
		DrawablePath stopActive;
		myPath.clear();
		myPath.addRectangle(0,0,12,12);
		stopPath.setStrokeThickness(3);
		stopPath.setStrokeFill(FillType(Colour(0xff0000aa)));
		stopPath.setPath(myPath);
		stopPath.setStrokeFill(FillType(Colour(0xff6666ff)));
		stopActive.setPath(myPath);
		
		addAndMakeVisible (stop = new DrawableButton (T("stop"), juce::DrawableButton::ImageOnButtonBackground));
		stop->addListener (this);
		stop->setBackgroundColours(Colour(0x77444444), Colour (0xaaaa0000));
		stop->setImages(&stopPath, 0, 0, 0, &stopActive, 0, 0, 0);
		                   
        myPath.clear();
		DrawablePath loadIcon; 
		myPath.addLineSegment(Line<float>(0,2,2,0), 1); 
		myPath.addLineSegment(Line<float>(2,0,5,0), 1); 
		myPath.addLineSegment(Line<float>(5,0, 7,2), 1); 
		myPath.addLineSegment(Line<float>(7,2,20,2), 1); 
		myPath.addLineSegment(Line<float>(20,2,20,20), 1); 
		myPath.addLineSegment(Line<float>(20,20,0,20), 1); 
		myPath.addLineSegment(Line<float>(0,20,0,2), 1);  
		
		loadIcon.setStrokeThickness(3); 
		loadIcon.setStrokeFill(FillType(Colours::lightgrey));
		loadIcon.setPath(myPath);
		
		addAndMakeVisible (load = new DrawableButton (T("load"), juce::DrawableButton::ImageOnButtonBackground ));
		load->setButtonText (T("load"));
		load->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight | Button::ConnectedOnTop | Button::ConnectedOnBottom);
		load->addListener (this);
		load->setBackgroundColours(Colours::transparentBlack, Colours::transparentBlack);
		load->setImages(&loadIcon, 0, 0, 0, &loadIcon, 0, 0, 0);
		
        
	}
	
	~AudioPlayerUI()
	{
		DBG("~AudioPlayerUI");
		deleteAllChildren();
	}
	
	void paint(Graphics& g);
 	
    void resized()
    {
        play->setBounds(10,20,30,30);
		stop->setBounds(50,22,25,25);
		load->setBounds(0,0,20,20);
    }
    
	void buttonClicked (Button* buttonThatWasClicked);

	
	void mouseDown(const MouseEvent &e);
	
	// drag and drop ...
	bool isInterestedInDragSource (const SourceDetails & details) {
		if (details.description == String("File Tree") ) return true;
		else return false;
	}
	
	void itemDragEnter (const SourceDetails & details) {
		
		
	}
	
	void itemDragMove (const SourceDetails & details) {
		
	}
	
	void itemDragExit (const SourceDetails & details) {}
	
	bool shouldDrawDragImageWhenOver () {return false;}
	
	void itemDropped (const SourceDetails & details) {
		
		if (details.description == String("File Tree"))
		{
			jassert(false);
			// try and load the file selected in this File Tree
			//myAudioPlayer->loadFile(dynamic_cast<FileTreeComponent*>(&details.sourceComponent)->getSelectedFile(0));
		}
		else if (File::isAbsolutePath(details.description) ) 
			myAudioPlayer->loadFile(File(details.description));
	}
	
};



//============================================================================== the UI
class AudioPlayerWaveformDisplay  : public Component,
public ButtonListener,
public SliderListener,
public DragAndDropTarget,
public ChangeListener,
public Timer
{
public:
	
	DrawableButton* play;
	DrawableButton* stop;
	DrawableButton* load;
	
	AudioPlayer* myAudioPlayer;
	
	//============================================================================== AudioPlayer Player
	AudioPlayerWaveformDisplay(AudioPlayer* _myAudioPlayer);
	
	~AudioPlayerWaveformDisplay() {
		DBG("~AudioPlayerUI");
		deleteAllChildren();
	}
	
	void paint(Graphics& g);
 	
	void resized();
	
	void buttonClicked (Button* buttonThatWasClicked);
	
	void sliderValueChanged(Slider* sliderThatWasMoved) {
		
	}
	
	void mouseDown(const MouseEvent &e);
	
	// drag and drop ...
	bool isInterestedInDragSource (const SourceDetails & details) {
		if (details.description == String("File Tree") ) return true;
		else return false;
	}
	
	void itemDragEnter (const SourceDetails & details) {
		
		
	}
	
	void itemDragMove (const SourceDetails & details) {
		
	}
	
	void itemDragExit (const SourceDetails & details) {}
	
	bool shouldDrawDragImageWhenOver () {return false;}
	
	void itemDropped (const SourceDetails & details) {
		
		if (details.description == String("File Tree"))
		{
			// try and load the file selected in this File Tree
			//myAudioPlayer->loadFile(dynamic_cast<FileTreeComponent*>(&details.sourceComponent)->getSelectedFile(0));
		}
		else if (File::isAbsolutePath(details.description) ) 
			myAudioPlayer->loadFile(File(details.description));
	}
	
	void changeListenerCallback (ChangeBroadcaster* objectThatHasChanged) {
	 
        repaint();
	}
	
	void timerCallback() {
		
		repaint();
	}
};



 
#endif