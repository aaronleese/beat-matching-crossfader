#include <stdio.h>
#include <string.h>

//#include <gtest/gtest.h>

#include "juce_amalgamated.h"

#include "rec/base/scoped_ptr.h"

#ifdef __JUCE_MAC_AUDIOCDREADER_HELPERS_JUCEHEADER__

// Don't run this test unless we have the upated JUCE.

TEST(AudioCDReaderHelper, getElementForKey) {
  XmlDocument doc("<dict><key>foo</key><value>bar</value></dict>");
  scoped_ptr<XmlElement> xml(doc.getDocumentElement());
  EXPECT_TRUE(!getElementForKey(*xml, "bar"));
  EXPECT_EQ(getElementForKey(*xml, "foo")->getFirstChildElement()->getText(),
            "bar");
}

TEST(AudioCDReaderHelper, getIntValueForKey) {
  XmlDocument doc("<dict><key>foo</key><value>123</value></dict>");
  scoped_ptr<XmlElement> xml(doc.getDocumentElement());
  EXPECT_EQ(getIntValueForKey(*xml, "bar"), -1);
  EXPECT_EQ(getIntValueForKey(*xml, "foo"), 123);
}

namespace {

// This CD is Kate Bush's "Never Forever".  Interestingly enough, there are
// half-a-dozen versions of this album, only different by a few seconds on one
// track or another.
//
// http://www.freedb.org/freedb/rock/9608bd0b is the specific CD used here.

const int TRACK_COUNT = 11;

const int SPF = AudioCDReader::SAMPLES_PER_FRAME;

// The offset of each track from the start, in frames.
int TRACK_LENGTHS[TRACK_COUNT + 1] = {
    183 * SPF,
    15240 * SPF,
    28113 * SPF,
    44215 * SPF,
    61385 * SPF,
    80298 * SPF,
    99478 * SPF,
    114173 * SPF,
    126925 * SPF,
    130820 * SPF,
    144218 * SPF,
    16895 * SPF
};

const int CDDB_ID = 0x8a08ca0b;

}

TEST(AudioCDReaderHelper, getTrackOffsets) {
  Array<int> offsets;
  EXPECT_STREQ(NULL, getTrackOffsets(File("../../../data"), &offsets));

  EXPECT_EQ(offsets.size(), TRACK_COUNT + 1);

  for (int i = 0; i < TRACK_COUNT; ++i)
    EXPECT_EQ(offsets[i], TRACK_LENGTHS[i]);
}

#if 0
TEST(AudioCDReaderHelper, CDDBIdComputation) {
  int id;
  EXPECT_STREQ(NULL, getCDDBId(File("../../../data"), &id));
  EXPECT_EQ(CDDB_ID, id) << std::hex << CDDB_ID << ", " << id
                         << "\n" << std::dec;
}

TEST(AudioCDReaderHelper, CDDBIdEndToEnd) {
  int id = getCDDBId(Array<int>(TRACK_LENGTHS, TRACK_COUNT + 1));
  EXPECT_EQ(CDDB_ID, id) << std::hex << CDDB_ID << ", " << id
                         << "\n" << std::dec;
}
#endif

#endif
