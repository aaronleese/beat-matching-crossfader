// Here's the structure of a sample Mac OS/X .TOC.plist file.
//
// <plist version="1.0">
// <dict>
//
// [other key/data pairs here]
//
// 	<key>Sessions</key>
// 	<array>
//     <dict>
//
//       [other key/data pairs that we don't care about here]
//
//       <key>Track Array</key>
//       <array>
//         <dict>
//
//          [other key/data pairs here]
//
//           <key>Start Block</key>
//           <integer>150</integer>
//         </dict>
//
//         <dict>
//
//          [other key/data pairs here]
//
//           <key>Start Block</key>
//           <integer>2029</integer>
//         </dict>
//
//        [more dicts here]
//
//       </array>
//     </dict>
//   </array>
// </dict>
// </plist>

// Apple's plist format uses XML in an unusual fashion as a sort of lame
// imitation of JSON - but why not just use JSON, or even better, its superset
// YAML?
//
// Their format defeats the whole purpose of XML, where your meaning is put in
// the tag names, by having meaningless tag names and having the meaning hidden
// in the value where neither your XPath, your XSLT nor your XML schema can get
// to it.
//
// An idiomatic XML document for the one above might look like:
//
// <sessions>
//   <trackArray>
//     <track>
//       <startBlock=150/>
//     </track>
//   </trackArray>
// </sessions>
//
// Now isn't that so much clearer?  Let's look at a subpart...
//
//   <trackArray><track><startBlock=150/></track></trackArray>
//
// vs Apple's
//
//   <key>Track Array</key>
//   <array><dict><key>Start Block</key><integer>150</integer></dict></array>
//
// Let's compare JSON (json.org)
//
//   {"trackArray": [{"startBlock": 150}]}
//
// Much nicer, yes?
//
// JSON is a subset of YAML 1.2, but you could also write it in YAML (see yaml.org) as:
//
//   trackArray:
//     -
//       startBlock: 150
//
// which seems a little cryptic until you see a longer document...
//
//   sessions:
//     -
//       first track: 1
//       last track: 11
//       leadout block: 168953
//       track array:
//         -
//           startBlock: 150
//           data: false
//           point: 1
//         -
//           startBlock: 15240
//           data: false
//           point: 2
//
// See how well it exposes just the data with whitespace and compare it to the
// .TOC.plist in this directory!  And you're always free to use the JSON form
// even within one file.

inline const XmlElement* getElementForKey(const XmlElement& xml,
                                          const String& key) {
    forEachXmlChildElementWithTagName(xml, child, "key")
    {
      if (child->getAllSubText() == key)
          return child->getNextElement();
    }
    return NULL;
}

inline int getIntValueForKey(const XmlElement& xml,
                             const String& key,
                             int dflt) {
    const XmlElement* block = getElementForKey(xml, key);
    return block ? int(strtol(block->getAllSubText().toCString(), NULL, 10)) : dflt;
}

inline const XmlElement* getFirstNamedElement(const XmlElement& xml, const String& name) {
    forEachXmlChildElementWithTagName(xml, child, name)
         return child;
    return NULL;
}

// Get the track offsets for a CD given an XmlElement representing its TOC.Plist.
// Returns NULL on success, otherwise a const char* representing an error.
inline const char* getTrackOffsets(XmlDocument* xmlDocument,
                                   Array<int>* offsets) {
    ScopedPointer<XmlElement> xml(xmlDocument->getDocumentElement());
    if (!xml)
        return "Couldn't parse XML in file";

    const XmlElement* dict = getFirstNamedElement(*xml, "dict");
    if (!dict)
        return "Couldn't get top level dictionary";

    const XmlElement* sessions = getElementForKey(*dict, "Sessions");
    if (!sessions)
        return "Couldn't find sessions key";

    const XmlElement* session = sessions->getFirstChildElement();
    if (!session)
        return "Couldn't find first session";

    int leadOut = getIntValueForKey(*session, "Leadout Block");
    if (leadOut < 0)
        return "Couldn't find Leadout Block";

    const XmlElement* trackArray = getElementForKey(*session, "Track Array");
    if (!trackArray)
        return "Couldn't find Track Array";

    forEachXmlChildElement(*trackArray, track)
    {
        int trackValue = getIntValueForKey(*track, "Start Block");
        if (trackValue < 0)
            return "Couldn't find Start Block in the track";
        offsets->add(trackValue * AudioCDReader::SAMPLES_PER_FRAME);
    }

    offsets->add(leadOut * AudioCDReader::SAMPLES_PER_FRAME);
    return NULL;
}

// Get the track offsets for a CD given a file representing the root volume for
// that CD.
//
// Returns NULL on success, otherwise a const char* representing an error.
inline const char* getTrackOffsets(const File& volume,
                                   Array<int>* offsets) {
    File toc = volume.getChildFile(".TOC.plist");
    if (!toc.exists())
        return "No TOC file";

    XmlDocument doc(toc);
    return getTrackOffsets(&doc, offsets);
}
