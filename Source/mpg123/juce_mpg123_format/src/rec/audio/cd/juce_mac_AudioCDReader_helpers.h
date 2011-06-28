#ifndef __JUCE_MAC_AUDIOCDREADER_HELPERS_JUCEHEADER__
#define __JUCE_MAC_AUDIOCDREADER_HELPERS_JUCEHEADER__

#include <inttypes.h>

// Functions to handle Apple's ill-conceived plist XML format.

// Get an element from a key from apple's "dict" structure.
const XmlElement* getElementForKey(const XmlElement& xml,
                                   const String& key);

int getIntValueForKey(const XmlElement& xml,
                      const String& key,
                      int dflt = -1);

const XmlElement* getFirstNamedElement(const XmlElement& xml,
                                       const String& name);

// Get the track offsets for a CD given an XmlElement representing its TOC.Plist.
// Returns NULL on success, otherwise a const char* representing an error.
const char* getTrackOffsets(XmlDocument* xmlDocument, Array<int>* offsets);

// Get the track offsets for a CD given a file representing the root volume for
// that CD.
//
// Returns NULL on success, otherwise a const char* representing an error.
const char* getTrackOffsets(const File& volume, Array<int>* offsets);

#include "juce_mac_AudioCDReader_helpers_impl.h"

#endif  // __JUCE_MAC_AUDIOCDREADER_HELPERS_JUCEHEADER__
