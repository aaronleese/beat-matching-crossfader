#ifndef __REC_AUDIO_FORMAT_MPG123_TAGS__
#define __REC_AUDIO_FORMAT_MPG123_TAGS__

#include "rec/audio/format/mpg123/Mpg123.h"
#include "juce_amalgamated.h"

namespace rec {
namespace audio {
namespace format {
namespace mpg123 {

#include "libmpg123/mpg123.h"

// Read mp3 tags into a StringPairArray.
Error getMp3Tags(mpg123_handle* mh, StringPairArray* metadata);

}  // namespace mpg123
}  // namespace format
}  // namespace audio
}  // namespace rec

#endif // __REC_AUDIO_FORMAT_MPG123_TAGS__
