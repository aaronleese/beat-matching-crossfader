#include <stdio.h>
#include <string.h>

//#include <gtest/gtest.h>

#include "juce_amalgamated.h"
#include "rec/audio/format/mpg123/Mpg123.h"
#include "rec/base/scoped_ptr.h"

namespace rec {
namespace audio {
namespace format {
namespace mpg123 {

static const char* FILENAME = "../../../data/test1";

	/*
TEST(Reader, All) {
  scoped_ptr<AudioFormatReader> wav(getFileReader(String(FILENAME) + ".wav"));
  scoped_ptr<AudioFormatReader> mp3(getFileReader(String(FILENAME) + ".mp3"));

  EXPECT_EQ(wav->sampleRate, mp3->sampleRate);
  EXPECT_EQ(wav->bitsPerSample, mp3->bitsPerSample);
  EXPECT_EQ(wav->lengthInSamples, mp3->lengthInSamples);
  EXPECT_EQ(wav->numChannels, mp3->numChannels);
  EXPECT_EQ(wav->numChannels, 1);
  EXPECT_EQ(wav->usesFloatingPointData, mp3->usesFloatingPointData);

  const int CHUNK_SIZE = 1024;
  const int BITS_LOST = 18;  // Why not 16?

  typedef int Sample;

  Sample wavRootSamples[CHUNK_SIZE];
  Sample mp3RootSamples[CHUNK_SIZE];
  Sample* wavSamples[1] = {wavRootSamples};
  Sample* mp3Samples[1] = {mp3RootSamples};

  for (int64 i = 0; i < wav->lengthInSamples; i += CHUNK_SIZE) {
    int64 remains = wav->lengthInSamples - i;
    int length = (remains < CHUNK_SIZE) ? remains : CHUNK_SIZE;
    EXPECT_TRUE(wav->read(wavSamples, wav->numChannels, i, length, false));
    EXPECT_TRUE(mp3->read(mp3Samples, mp3->numChannels, i, length, false));
    for (int64 i = 0; i < length; i++)
      EXPECT_NEAR(wavRootSamples[i], mp3RootSamples[i], 0x30000);
  }
}
	 */

}  // namespace mpg123
}  // namespace format
}  // namespace audio
}  // namespace rec
