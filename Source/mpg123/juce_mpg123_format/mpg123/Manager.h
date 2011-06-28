#ifndef __REC_AUDIO_FORMAT_MPG123_MANAGER__
#define __REC_AUDIO_FORMAT_MPG123_MANAGER__

#include "audio/audio_file_formats/juce_AudioFormat.h"

#include "rec/audio/format/mpg123/Mpg123.h"
#include "rec/audio/format/mpg123/CreateReader.h"
#include "rec/audio/format/mpg123/NewHandle.h"
#include "rec/base/disallow.h"

namespace rec {
namespace audio {
namespace format {
namespace mpg123 {

class Format : public AudioFormat {
 public:
  Format() : AudioFormat(getTranslatedName(), getMp3FileExtensions()) {}

  ~Format() {}

  virtual AudioFormatReader* createReaderFor(InputStream* sourceStream,
                                             bool deleteStreamIfOpeningFails) {
    AudioFormatReader* reader = NULL;
    if (Error e = createReader(sourceStream, &reader)) {
      std::cerr << "mpg123 create error:" << mpg123_plain_strerror(e) << "\n";
      if (deleteStreamIfOpeningFails)
        delete sourceStream;
    }

    return reader;
  }

  virtual AudioFormatWriter* createWriterFor(OutputStream* streamToWriteTo,
                                             double sampleRateToUse,
                                             unsigned int numberOfChannels,
                                             int bitsPerSample,
                                             const StringPairArray& metadata,
                                             int qualityOptionIndex) {
    return NULL;
  }

  virtual const Array<int> getPossibleSampleRates() { return getSampleRates(); }
  virtual const Array<int> getPossibleBitDepths() { return getBitDepths(); }

  virtual bool canDoStereo() { return true; }
  virtual bool canDoMono() { return true; }
  virtual bool isCompressed() { return true; }

  juce_UseDebuggingNewOperator

 private:
  DISALLOW_COPY_AND_ASSIGN(Format);
};

}  // namespace mpg123
}  // namespace format
}  // namespace audio
}  // namespace rec

#endif // __REC_AUDIO_FORMAT_MPG123_MANAGER__
