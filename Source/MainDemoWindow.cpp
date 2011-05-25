/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "juceHeader.h"
#include "MainDemoWindow.h"

#include "AudioPlayer.h"

//==============================================================================
class ContentComp  : public Component, public SliderListener
{
    
    MainAudioProcessor* myAudioProcessor;
    Slider* crossfader;
    
    AudioPlayerUI* leftPlayer;
    AudioPlayerWaveformDisplay* waveformDisplay;
    AudioPlayerUI* rightPlayer;
    AudioPlayerWaveformDisplay* waveformDisplayR;
    
    
public:
    //==============================================================================
    ContentComp (MainDemoWindow* mainWindow_ ,  MainAudioProcessor* _myAudioProcessor)
        : mainWindow (mainWindow_)
    {
        
        myAudioProcessor = _myAudioProcessor;
        
        addAndMakeVisible(leftPlayer = new AudioPlayerUI(myAudioProcessor->myAudioPlayerLeft));
        
        addAndMakeVisible(waveformDisplay = new AudioPlayerWaveformDisplay(myAudioProcessor->myAudioPlayerLeft));
        
        addAndMakeVisible(rightPlayer = new AudioPlayerUI(myAudioProcessor->myAudioPlayerRight));
        
        addAndMakeVisible(waveformDisplayR = new AudioPlayerWaveformDisplay(myAudioProcessor->myAudioPlayerRight));
        
        
        
        addAndMakeVisible(crossfader = new Slider()); 
        crossfader->setRange(0, 1, .01);
        crossfader->setBounds(50, 200, 600, 30);
        crossfader->addListener(this);
                
    }

    ~ContentComp()
    {
        DBG("~contentComp");

        deleteAllChildren();
        
        
        
    }
    
    void sliderValueChanged(Slider* sliderThatWasMoved) {
		
        float crossfadePercentage = sliderThatWasMoved->getValue();
        
        myAudioProcessor->myAudioPlayerLeft->setVolume(1 - crossfadePercentage);
        myAudioProcessor->myAudioPlayerRight->setVolume(crossfadePercentage);
        
        
        // linear 
        myAudioProcessor->targetBpm = myAudioProcessor->myAudioPlayerLeft->bpm 
            + crossfadePercentage*(myAudioProcessor->myAudioPlayerRight->bpm - myAudioProcessor->myAudioPlayerLeft->bpm);
        
        
        float bpmShift1 =  myAudioProcessor->targetBpm/myAudioProcessor->myAudioPlayerLeft->bpm;
        float bpmShift2 =  myAudioProcessor->targetBpm/myAudioProcessor->myAudioPlayerRight->bpm;
        
        // now tweak slightly to get the individual beats to line up ...
        //int nextBeatL = myAudioProcessor->myAudioPlayerLeft->getSamplesToNextBeat();
        
        //DBG("adsasdf"  + String(nextBeatL));
        
        // set the timeStretching for each player ...
        myAudioProcessor->myAudioPlayerLeft->timeStretch->setValue(bpmShift1);
        myAudioProcessor->myAudioPlayerRight->timeStretch->setValue(bpmShift2);
        
	}
    
    void paint(Graphics & g)
    {
        g.fillAll(Colours::darkgrey);
    }
    
    
    //==============================================================================
    void resized()
    {
      
        leftPlayer->setBounds(40,40,300,100);
        rightPlayer->setBounds(360,40, 300,100);
        
        waveformDisplay->setBounds(50, 300, 600, 100);
        waveformDisplayR->setBounds(50, 400, 600, 100);
        
    }

    //==============================================================================
    // The following methods implement the ApplicationCommandTarget interface, allowing
    // this window to publish a set of actions it can perform, and which can be mapped
    // onto menus, keypresses, etc.


private:
    //==============================================================================
    MainDemoWindow* mainWindow;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentComp);
};


//==============================================================================
MainDemoWindow::MainDemoWindow(MainAudioProcessor* _myAudioProcessor) : DocumentWindow ("Echonest Crossfade",
                      Colours::black,
                      DocumentWindow::allButtons,
                      true)
{
    setResizable (true, false); // resizability is a property of ResizableWindow
    setResizeLimits (400, 300, 8192, 8192);

    setContentOwned(new ContentComp(this, _myAudioProcessor), false);
    
    setVisible (true);
 
}

MainDemoWindow::~MainDemoWindow()
{
    DBG("~mainDemoWindow"); 
    
}

void MainDemoWindow::closeButtonPressed()
{
    // The correct thing to do when you want the app to quit is to call the
    // JUCEApplication::systemRequestedQuit() method.

    // That means that requests to quit that come from your own UI, or from other
    // OS-specific sources (e.g. the dock menu on the mac) all get handled in the
    // same way.

    JUCEApplication::getInstance()->systemRequestedQuit();
}
