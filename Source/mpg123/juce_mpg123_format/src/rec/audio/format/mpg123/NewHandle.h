#ifndef __REC_AUDIO_FORMAT_MPG123_NEW_HANDLE__
#define __REC_AUDIO_FORMAT_MPG123_NEW_HANDLE__

#include "rec/audio/format/mpg123/Mpg123.h"

#include "juce_amalgamated.h"

namespace rec {
namespace audio {
namespace format {
namespace mpg123 {

// Create a new mpg123_handle for a Juce InputStream.
Error newHandle(InputStream* in, mpg123_handle** mh);

}  // namespace mpg123
}  // namespace format
}  // namespace audio
}  // namespace rec

#endif  // __REC_AUDIO_FORMAT_MPG123_NEW_HANDLE__
