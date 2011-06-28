#include <stdio.h>

//#include <gtest/gtest.h>

#include "rec/audio/format/mpg123/Mpg123.h"

namespace rec {
namespace audio {
namespace format {
namespace mpg123 {

static const char* FILENAME = "../../../data/test1";

	/*
TEST(CreateReader, Create) {
  AudioFormatReader *reader = getFileReader(String(FILENAME) + ".mp3");
  ASSERT_FALSE(!reader);
  StringPairArray& data = reader->metadataValues;

  EXPECT_EQ(data["idversion"], "id3v2");
  EXPECT_EQ(data["TENC"], "Amadeus Pro");
  EXPECT_EQ(data["TIT2"], "The Title");
  EXPECT_EQ(data["TALB"], "The Album");
  EXPECT_EQ(data["TPE1"], "The Artist");
  EXPECT_EQ(data["TYER"], "2010");
  EXPECT_EQ(data["TRCK"], "2");
  EXPECT_EQ(data["TCON"], "Space punk");
  EXPECT_EQ(data["COMM"], "This is a comment.");

  delete reader;
}
*/
	
	
}  // namespace mpg123
}  // namespace format
}  // namespace audio
}  // namespace rec
