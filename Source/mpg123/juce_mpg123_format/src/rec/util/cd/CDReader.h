#ifndef __REC_UTIL_CD_CDREADER__
#define __REC_UTIL_CD_CDREADER__

//#include "rec/base/base.h"

namespace rec {
namespace util {
namespace cd {

// Audio CDs have both data and audio tracks, which may appear in any order.
//
// For example, an audio CD might contain 6 tracks, where tracks 1 and 3 are
// data tracks and tracks 0, 2, 4, and 5 are audio:  A D A D A A
//
// Here, getAudioTrackCount(reader, 0) returns 4 - 4 audio tracks./
// getAudioTrackCount(reader, 0) returns 0, but
// getAudioTrackCount(reader, 1) returns 2 (because track 1 is data), and
// getAudioTrackCount(reader, 3) returns 5

// Return a new AudioCDReader for the CD with the given CDDB id, or NULL if no
// such CD exists in a CD drive.
AudioCDReader* getAudioCDReader(const String& cddbId);

// Return a new AudioFormatReader for an audio cd track by audio track index.
AudioFormatReader* createCDTrackReader(AudioCDReader* reader, int track);
AudioFormatReader* createCDTrackReader(const String& cddbId, int track);

// Return the number of audio tracks on an audio CD.
int getAudioTrackCount(const AudioCDReader& reader);

// Given the index for an audio track on a CD, returns the index for that track
// in the list of all tracks, data and audio.
int getAudioTrackIndex(const AudioCDReader& reader, int track);

}  // namespace cd
}  // namespace util
}  // namespace rec

#endif  // __REC_UTIL_CD_CDREADER__
