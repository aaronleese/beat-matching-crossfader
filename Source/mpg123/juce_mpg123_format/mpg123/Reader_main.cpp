#include <stdio.h>

//#include <gtest/gtest.h>

#include "juce_amalgamated.h"

#include "rec/audio/format/mpg123/CreateReader.h"
#include "rec/audio/format/mpg123/Mpg123.h"
#include "rec/audio/format/mpg123/Format.h"
#include "rec/base/scoped_ptr.h"

using rec::audio::format::mpg123::Format;

/*
int main(int argc, char * const argv[]) {
  if (argc > 3) {
    std::cerr << argv[0] << " Usage: Reader filein fileout\n";
    return -1;
  }

  const char* inName = (argc > 1) ? argv[1] : "../../../data/test1.mp3";

  File infile(inName);
  if (!infile.exists()) {
    std::cerr << "File " << inName << " doesn't exist\n";
    return -1;
  }

  InputStream* in = infile.createInputStream();
  if (!in) {
    std::cerr << "Couldn't open input stream for " << inName << "\n";
    return -1;
  }

  scoped_ptr<AudioFormatReader> reader(Format().createReaderFor(in, true));
  if (!reader) {
    std::cerr << "File " << inName << " doesn't seem to be an mp3 file\n";
    return -1;
  }

  const char* outName = (argc > 2) ? argv[2] : "../../../data/test1.wav";
  
  File outfile(outName);
  if (outfile.exists()) {
    if (!outfile.deleteFile())
      std::cerr << "File " << outName << " couldn't be deleted.\n";
    return -1;
  }

  OutputStream* outStream = outfile.createOutputStream();
  if (!outStream) {
    std::cerr << "Couldn't open output stream for " << outName << "\n";
    return -1;
  }

  scoped_ptr<AudioFormatWriter> writer(
      WavAudioFormat().createWriterFor(outStream,
                                       reader->sampleRate,
                                       reader->numChannels,
                                       reader->bitsPerSample,
                                       reader->metadataValues,
                                       0));

  if (!writer) {
    std::cerr << "Couldn't open writer for " << outName << "\n";
    return -1;
  }

  if (!writer->writeFromAudioReader(*reader, 0, reader->lengthInSamples)) {
    std::cerr << "Couldn't write for " << outName << "\n";
    return -1;
  }

  std::cout << "Wrote " << argv[1] << " to " << outName << "\n";
  return 0;
}
 
 */
