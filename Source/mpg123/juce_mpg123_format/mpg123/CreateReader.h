#ifndef __REC_AUDIO_FORMAT_MPG123_CREATE_READER__
#define __REC_AUDIO_FORMAT_MPG123_CREATE_READER__

#include "juce_amalgamated.h"
#include "src/io/streams/juce_InputStream.h"
#include "src/audio/audio_file_formats/juce_AudioFormat.h"

#include "rec/audio/format/mpg123/Mpg123.h"

namespace rec {
namespace audio {
namespace format {
namespace mpg123 {

// Represents a specific rate/channel/encoding we accept.
struct OutputFormat {
  long rate_;
  int channels_;
  mpg123_enc_enum encoding_;
};

// Create a reader from an InputStream, or return an error.  begin/end form a
// list of OutputFormats that we want to accept - if they're NULL, or the same,
// we accept all formats.
Error createReader(InputStream* sourceStream,
                   AudioFormatReader** reader,
                   OutputFormat* begin = NULL,
                   OutputFormat* end = NULL);

}  // namespace mpg123
}  // namespace format
}  // namespace audio
}  // namespace rec

#endif // __REC_AUDIO_FORMAT_MPG123_CREATE_READER__
