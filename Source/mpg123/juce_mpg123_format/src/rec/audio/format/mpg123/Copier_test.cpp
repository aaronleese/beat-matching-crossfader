#include <vector>

//#include <gtest/gtest.h>

#include "rec/audio/format/mpg123/Copier.h"
#include "rec/audio/format/mpg123/Mpg123.h"

namespace rec {
namespace audio {
namespace format {
namespace mpg123 {
  
typedef unsigned char uchar;

template <typename In, typename Out>
void testCopier(int encoding, Out mult = 1, Out offset = 0) {
  In samples[3][2] = {{1, 0x11}, {2, 0x12}, {3, 0x13}};
  Out destBase[2][3];
  Out* dest[2] = {destBase[0], destBase[1]};
  getCopier(encoding)(reinterpret_cast<int**>(dest), 2, 0, samples[0], 2, 3);

  for (int c = 0; c < 2; ++c) {
    for (int s = 0; s < 3; ++s) {
      ASSERT_EQ(Out(mult * (int64(samples[s][c]) - offset)), destBase[c][s])
        << c << ", " << s;
    }
  }
}

	// google unit testing ..
	/*
TEST(RecAudioFormatMpg123Copier, Signed) {
  testCopier<int, int>(MPG123_ENC_SIGNED_32);
  testCopier<short, int>(MPG123_ENC_SIGNED_16, 0x10000);
  testCopier<char, int>(MPG123_ENC_SIGNED_8, 0x1000000);
}

TEST(RecAudioFormatMpg123Copier, Float) {
  testCopier<float, float>(MPG123_ENC_FLOAT_32);
  testCopier<double, float>(MPG123_ENC_FLOAT_64);
}

TEST(RecAudioFormatMpg123Copier, Unsigned) {
  testCopier<uint, int>(MPG123_ENC_UNSIGNED_32, 1, 0x80000000L);
  testCopier<ushort, int>(MPG123_ENC_UNSIGNED_16, 0x10000, 0x8000);
  testCopier<uchar, int>(MPG123_ENC_UNSIGNED_8, 0x1000000, 0x80);
}
*/
	
}  // namespace mpg123
}  // namespace format
}  // namespace audio
}  // namespace rec
