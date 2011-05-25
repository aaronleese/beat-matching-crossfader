/*
 *  AudioPlayer.cpp
 *  Livetronica Studio
 *
 *  Created by Aaron Leese on 4/4/10.
 *  Copyright 2010 StageCraft Software. All rights reserved.
 *
 */


#include "AudioPlayer.h" 
 
#include "Mpg123.h"
#include "Format.h" 

#include "Codegen.h"

AudioFormatManager* formatManager;

////////////////////////////////////////////////////////////// AudioPlayer
AudioPlayer::AudioPlayer() : Thread("file reading thread") {
	currentBeatIndex = 0;
    
    myReader = 0;
    
    volume = 1;
    bpm = 0;
    
	playing = false;
	
	thumbImage = new Image(juce::Image::RGB, 220, 50, true);
	thumbGraphic = new Graphics(*thumbImage);
	 
	reader = 0;
	playPos = 0;
	
	trackBuffer = bufferProcessed = 0;
	
    myTrackInformation = 0;
	
    if (formatManager == 0) formatManager = new AudioFormatManager();
    
    if (formatManager->getNumKnownFormats() == 0)
        formatManager->registerBasicFormats();

    formatManager->registerFormat(new rec::audio::format::mpg123::Format(), false);	
    
    
    audioThumbCache = new AudioThumbnailCache(10);
	audioThumb = new AudioThumbnail(512, *formatManager, *audioThumbCache); 	

	// run time init, after bufferSize is determined ... resize the AudioPlayerBuffer appropriately ..
	trackBuffer = new juce::AudioSampleBuffer(2, 1000);
	trackBuffer->clear();
	
	bufferProcessed = new juce::AudioSampleBuffer(2, 512);
    
    // INIT SOUNDTOUCH
    setInputBuffer(trackBuffer);
    
}

AudioPlayer::~AudioPlayer() {
	
	DBG (String("~AudioPlayer()"));
	
	delete thumbImage;
	delete thumbGraphic;
	
	delete audioThumbCache;
	
	if (audioThumb != 0) delete audioThumb;
	
	if (reader != 0) delete reader;
	
    if (myTrackInformation != 0) delete myTrackInformation;
   
    if (myReader == 0) deleteAndZero(myReader);
    if (formatManager != 0) deleteAndZero(formatManager);
    
	if (bufferProcessed != 0) delete bufferProcessed;
	if (trackBuffer != 0) delete trackBuffer; 	
	
}
 
void AudioPlayer::shutdown() {
	
	DBG(String("shutting down track")) ;
	
	stopTimer();
	
	playing = false;
	
}
 
void AudioPlayer::timerCallback() {
	 
	 
}

void AudioPlayer::loadFile(const File &fileToLoad) {
	 
	DBG ("loading .... "); 

	playing = false;
	
	trackBuffer->clear();
	playPos = 0;
	
	DBG ("AudioPlayer - loading audio");
	
	if ( ! fileToLoad.existsAsFile()) return; 
	
	if (reader != 0) delete reader;
	reader = formatManager->createReaderFor(fileToLoad);
	if (reader == 0) return;
	
	trackName = fileToLoad.getFileNameWithoutExtension();
	String loadedFilePath = fileToLoad.getFullPathName(); 		
	
	freqShiftBase->setValue(1);
	
	trackBuffer->setSize(2, reader->lengthInSamples, false, true); // NOTE ***** the 1 ... we are condensing to 1 channel
	trackBuffer->clear();
	
	AudioPlayer::startThread(2);
	
    
	// get the thumbnail ...
	audioThumb->reset(2, 44100);
	audioThumb->setSource(new FileInputSource (fileToLoad));
	 
	DBG("ID3 tag: " + String(reader->metadataValues.getDescription()));
	
	String title = reader->metadataValues.getValue("TIT2", "untitled");
	String artist = reader->metadataValues.getValue ("TPE1", "unknown");
	String album = reader->metadataValues.getValue ("TALB", "unknown");
	
	// ADD to the library .....
	
    if (myTrackInformation != 0) delete myTrackInformation;
    myTrackInformation = new ValueTree("Track");

    myTrackInformation->setProperty(String("name"), fileToLoad.getFileNameWithoutExtension(), 0);
	myTrackInformation->setProperty(String("path"), fileToLoad.getFullPathName(), 0);
	myTrackInformation->setProperty(String("title"), title, 0);
	myTrackInformation->setProperty(String("artist"), artist, 0);
	myTrackInformation->setProperty(String("album"), album, 0);

    analyzeTrack();
    
	DBG ("AudioPlayer - done loading audio");
		
    
}

void AudioPlayer::analyzeTrack()
{

    jassert(trackBuffer != 0);
    
    
    // Codegen(const float* pcm, uint numSamples, int start_offset, bool low_rank);
    //Codegen * pCodegen = new Codegen(trackBuffer->getSampleData(0,0), trackBuffer->getNumSamples(), 0, false);

    //String fp = pCodegen->getCodeString().c_str();
    //char* fp = pCodegen->getCodeString();
    //String fp = pCodegen->getCodeString().c_str();
    
    String artist = myTrackInformation->getProperty("artist").toString();
    String title = myTrackInformation->getProperty("title").toString();
    
    float duration = float(trackBuffer->getNumSamples())/44100.0f;
    
    // inserted into the URL to avoid caching issues ...
    Random rand(Time::currentTimeMillis());
    
    String urlString = "http://" + String(rand.nextInt(100)) + "@developer.echonest.com/api/v4/song/search?api_key=7GPG0NNLAFLB9EEEX"
        + String("&artist=") +  artist
        + String("&title=") + title
        + String("&min_duration=" + String(duration - 0.1))
        // + String("&max_duration=" + String(duration + 0.5))
        + String("&bucket=audio_summary")
        + String("&results=4")
        + String("&format=xml")
       // + String(";" + String(rand.nextInt(100)))
        // + String("&code=") + String(fp)
    ;

    
    DBG("URL: " + urlString);
    URL identify( urlString );
    
    URL searchString( urlString );
    InputStream* searchStream = searchString.createInputStream(false, 0, 0);
    if (searchStream == 0) return;
    
    XmlElement* results = XmlDocument::parse(searchStream->readEntireStreamAsString());
    
    if (results == 0) 
    {
        DBG("no results");
        return;
    }
    
    
    XmlElement* songs = results->getChildByName("songs");
    
    if (songs == 0) return;
    
    forEachXmlChildElement (*songs, song)
    {
        
        XmlElement* artist_name = song->getChildByName("artist_name");
        
        if (artist == artist_name->getAllSubText())
        {
            
            DBG("a match");
            // lets try an analysis ....
            
            XmlElement* summary = song->getChildByName("audio_summary");
            jassert(summary != 0);
            
            
            XmlElement* analysis_url = summary->getChildByName("analysis_url");
            jassert(analysis_url != 0);
           
            // some other high level meta data ...
            XmlElement* key = summary->getChildByName("key");
            if (key != 0) myTrackInformation->setProperty(String("key"), key->getAllSubText(), 0);
            
            XmlElement* tempo = summary->getChildByName("tempo");
             if (tempo != 0) myTrackInformation->setProperty(String("tempo"), tempo->getAllSubText(), 0);
            
            XmlElement* duration = summary->getChildByName("duration");
            if (duration != 0) myTrackInformation->setProperty(String("duration"), duration->getAllSubText(), 0);
            
    
            URL analysisURL( analysis_url->getAllSubText() );
            InputStream* analysisStream = analysisURL.createInputStream(false, 0);
            if (analysisStream == 0) return;
            readJson(analysisStream->readEntireStreamAsString());     
            delete analysisStream; 
    
            break;
        }
         
    }
    
    
    delete results;
    delete searchStream;
    
    // some other echonest calls ...
    
    // InputStream* identifyStream = identify.createInputStream(false, 0);
   // if (identifyStream == 0) return;
  //  DBG("echo - identify: "  + String(identifyStream->readEntireStreamAsString()) );
    
    
   // URL analysis("http://developer.echonest.com/api/v4/track/analyze?api_key=7GPG0NNLAFLB9EEEX&id=" + fp);
    
  //  InputStream* analysisStream = analysis.createInputStream(false, 0);
  //  if (analysisStream == 0) return;
   // DBG("echo - analyse: "  + String(analysisStream->readEntireStreamAsString()) );
    
    
    
}

void AudioPlayer::run() // worker thread for loading files ...
{
	
	jassert(trackBuffer->getNumSamples() > 200000);
	
	BPMShifter::setInputBuffer(trackBuffer);
	
	trackBuffer->readFromAudioReader(reader, 0, 200000, 0, true, true); 
	
    currentBeatIndex = 0;
	playing = true;
    
    trackBuffer->readFromAudioReader(reader, 200000, reader->lengthInSamples-200000, 200000, true, true); 

	
}

int AudioPlayer::getSamplesToNextBeat()
{
   // DBG("current " + String(currentBeatIndex));
    
    if (myTrackInformation == 0) return 0;
    if (myReader == 0) return 0;
    if (myReader->isThreadRunning()) return 0;
    
    
    ValueTree beats = myTrackInformation->getChildWithName("beats");
      
    float nextBeat = beats.getChild(currentBeatIndex).getChildWithName("start").getProperty("value", 0);
     
    //if ( ! beat.isValid()) return 0;
   
    if (nextBeat <= 0) return 0; // invalid ...
    
    while (nextBeat < playPos/44100.0f)
    {
        currentBeatIndex++;
        nextBeat = beats.getChild(currentBeatIndex).getChildWithName("start").getProperty("value", 0);
        
    }
     
    
    return nextBeat*44100 - playPos;
    
}

Component* AudioPlayer::getEditor() {  
    
	return new AudioPlayerUI(this);
}

/////////////////////////////////////////////// ADUIO CALLBACK FUNCTIONS

void AudioPlayer::calculateCurrentFreq() {	
    
    freqShiftCurrent = (float)freqShiftBase->getValue();  
	
} 

void AudioPlayer::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages) {
	
	if ( ! playing) return;
	
	int numSamples = buffer.getNumSamples();
	
	// Calc V 
	calculateCurrentFreq();
	BPMShifter::processSoundTouchBlockWithGain(buffer, playPos, volume);
	float freqShift_total = rate*tempo*freqShiftCurrent; // linearly interpolated ... so moves the 
    
	// move the play position in the cache ... so we get different data next time we process
	playPos = int(playPos + float(numSamples)*freqShift_total + trackBuffer->getNumSamples())%trackBuffer->getNumSamples();
	
    
    if (myTrackInformation == 0) return;
    
    ValueTree beats = myTrackInformation->getChildWithName("beats");
    
    if (freqShift_total <= 0) return;
    
}

/////////////////////////////////////////////// PLAYER UI
void AudioPlayerUI::paint(Graphics& g) {
	 
     
    g.setGradientFill(ColourGradient(Colours::darkgoldenrod, 0, 0, Colours::black, getWidth(), getHeight(), true));
    
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 10);
	
}

void AudioPlayerUI::buttonClicked (Button* buttonThatWasClicked) {
	
    if (buttonThatWasClicked == play)
	{
		myAudioPlayer->playPos = 0; 
		myAudioPlayer->playing = true;
	}
	else if (buttonThatWasClicked == load)
	{
        
       
		WildcardFileFilter wildcard(formatManager->getWildcardForAllFormats(), String::empty, String("Audio Files"));
		
		FileBrowserComponent browser (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, File::nonexistent, &wildcard, 0);
		
		FileChooserDialogBox dialogBox (T("Open Audio File"),
										T("Select an audio file to play on this turntable."),
										browser, true,
										Colours::white);
		
		if (dialogBox.show())
			myAudioPlayer->loadFile(browser.getSelectedFile(0));
		
		
	}
	else if (buttonThatWasClicked == stop)
	{
		myAudioPlayer->playing = false;
	}
	
}

void AudioPlayerUI::mouseDown(const MouseEvent &e) { 
	
}


/////////////////////////////////////////////// WAVEFORM DISPLAY

AudioPlayerWaveformDisplay::AudioPlayerWaveformDisplay(AudioPlayer* _myAudioPlayer) {
	
	DBG (String("new audio player UI"));
	myAudioPlayer = _myAudioPlayer;
	 
    
	myAudioPlayer->audioThumb->addChangeListener(this);
	
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
	play->setBounds(10,20,30,30);
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
	stop->setBounds(50,22,25,25);
	
	
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
	
	play->setSize(0,0);
	stop->setSize(0,0);
	
	startTimer(1000/20);
	
	DBG (String("TTAudioTrack() complete"));
	
}

void AudioPlayerWaveformDisplay::resized() {
	 
    
	
}

void AudioPlayerWaveformDisplay::paint(Graphics& g) {
	
    g.setGradientFill(ColourGradient(Colours::darkblue, 0, 0, Colours::black, getWidth(), getHeight(), true));
    g.fillRect(0, 0, getWidth(), getHeight());
    
	g.setColour(Colours::dimgrey);
	  
	float playTime = float(myAudioPlayer->playPos)/44100.0f; // seconds
	
	myAudioPlayer->audioThumb->drawChannel(g, Rectangle< int >(0,0, getWidth(), getHeight()*2), playTime-5, playTime+5, 0, 1);
 	
    
    if (myAudioPlayer->myReader != 0)
    {
        // currently reading ....
        
        if (myAudioPlayer->myReader->isThreadRunning()) 
        {
            DBG("reading ... ");
            return;
        }
    }
    
    if (myAudioPlayer->myTrackInformation == 0) return;
    
    // green playHead marker line 
    g.setColour(Colours::green);
    g.drawLine(getWidth()/2, 0, getWidth()/2, getHeight(), 1);
    
    // BARS
    g.setColour(Colours::white);
    ValueTree trackBars = myAudioPlayer->myTrackInformation->getChildWithName("bars");
    //DBG("bars : "+ String(trackBars.getNumChildren()) + " " + String(playTime));
    for (short i=0; i < trackBars.getNumChildren(); i++)
    {
        ValueTree bar = trackBars.getChild(i);
        
        if (bar.getChildWithName("start").isValid())
        {                 
            ValueTree start = bar.getChildWithName("start");
            
          //  DBG("bar " + String(i) + " " + start.getProperty("value").toString());
              
            if (float(start.getProperty("value")) < playTime - 5) continue;
            else if (float(start.getProperty("value")) > playTime + 5) break;
            else {
                
              //  DBG(start.getProperty("value").toString() );
                
                float marker = start.getProperty("value");
                
                int offset = getWidth()*((marker - playTime + 5) / (10));
                
                g.drawLine(offset, 0, offset, getHeight());
                
            }     
            
        }
        
        
    }
    
    
    // BEATS
    ValueTree trackBeats = myAudioPlayer->myTrackInformation->getChildWithName("beats");
    
    g.setColour(Colours::lightgrey);
    for (short i=0; i < trackBeats.getNumChildren(); i++)
    {
        ValueTree beat = trackBeats.getChild(i);
        
        if (beat.getChildWithName("start").isValid())
        {                 
            ValueTree start = beat.getChildWithName("start");
            
            if (float(start.getProperty("value")) < playTime - 5) continue;
            else if (float(start.getProperty("value")) > playTime + 5) break;
            else {
                
                if (myAudioPlayer->currentBeatIndex == i)
                    g.setColour(Colours::orange);
                else if (myAudioPlayer->currentBeatIndex + 1 == i)
                    g.setColour(Colours::yellowgreen);
                
                // get the confidence 
                float confidence = beat.getChildWithName("confidence").getProperty("value");
                float marker = start.getProperty("value");
                int offset = getWidth()*((marker - playTime + 5) / (10));
                
                g.drawLine(offset, getHeight() - getHeight()*confidence, offset, getHeight());
                
            }     
            
        }
        
    }
             
    ValueTree trackInfo = myAudioPlayer->myTrackInformation->getChildWithName("track");
    
    // Aaron - better ways surely exist to do this ...
    if (trackInfo.isValid())
    {
        if (trackInfo.getChildWithName("tempo").isValid())
            myAudioPlayer->bpm = trackInfo.getChildWithName("tempo").getProperty("value");
    
        /*
        if (trackInfo.getChildWithName("mode").isValid())
            myAudioPlayer->myTrackInformation->setProperty("mode", trackInfo.getChildWithName("mode").getProperty("value"), 0);
    
        if (trackInfo.getChildWithName("time_signature").isValid())
            myAudioPlayer->myTrackInformation->setProperty("time_signature", trackInfo.getChildWithName("time_signature").getProperty("value"), 0);
        
        */
        
        g.setColour(Colours::lightgreen);
        g.drawText("bpm: " + String(myAudioPlayer->bpm) + "->" + String(myAudioPlayer->bpm*(float)(myAudioPlayer->timeStretch->getValue())) , 0, 0, 300, 20, Justification::left, false);
        
        g.drawText("key: " + String(myAudioPlayer->myTrackInformation->getProperty("key").toString()) 
                   + String(myAudioPlayer->myTrackInformation->getProperty("mode").toString())
                   , 0, 20, 100, 20, Justification::left, false);
        
        g.drawText("length: " + String(myAudioPlayer->myTrackInformation->getProperty("duration").toString()), 0, 40, 100, 20, Justification::left, false);
        
        g.drawText("time sig: " + String(myAudioPlayer->myTrackInformation->getProperty("time_signature").toString()), 0, 60, 100, 20, Justification::left, false);
        

    }
    
 
    
}

void AudioPlayerWaveformDisplay::buttonClicked (Button* buttonThatWasClicked) {
	
    }

void AudioPlayerWaveformDisplay::mouseDown(const MouseEvent &e) { 
	
}

/////////////////////////////////////////////// JSON

void AudioPlayer::readJson(String jsonStream)
{
    
    //JsonToJuceReader* 
    
    if (myReader == 0) myReader = new JsonToJuceReader();
    myReader->ReadJsonToValueTree(jsonStream, myTrackInformation);

    // delete myReader;
        
    
    //ValueTree trackInfo = myTrackInformation->getChildWithName("track");
     
    /*
     Json::Reader* myJsonReader = new Json::Reader();
     std::string stdJsonString(jsonStream.toUTF8()); 
     Json::Value rootValue =  new Json::Value();
     
     bool success = myJsonReader->parse(stdJsonString, rootValue);
     
     DBG("success? " + String(success));
     
     
     // PrintJSONTree(rootValue, 0);
     ReadJSONToValueTree(rootValue, *myTrackInformation, 0); 
     
     DBG("kids " + String(myTrackInformation->getNumChildren()));
     
     */
    
    // set bpm
    
    
    /*
    if (trackInfo.getChildWithName("tempo").isValid())
        bpm = trackInfo.getChildWithName("tempo").getProperty("value");
    
    if (trackInfo.getChildWithName("mode").isValid())
        myTrackInformation->setProperty("mode", trackInfo.getChildWithName("mode").getProperty("value"), 0);
    
    if (trackInfo.getChildWithName("time_signature").isValid())
        myTrackInformation->setProperty("time_signature", trackInfo.getChildWithName("time_signature").getProperty("value"), 0);
    */
    
    
    
    
}




