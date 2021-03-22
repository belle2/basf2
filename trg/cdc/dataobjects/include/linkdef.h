
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;


// ROOT dictionaries will be build for all classes mentioned this way
#pragma link C++ class bitset <639>+; // checksum=0x88f24d, version=2
#pragma link C++ class bitset <256>+; // checksum=0x88f1ea, version=2
#pragma link C++ class Belle2::CDCTriggerSegmentHit+; // checksum=0x89ed4679, version=4
#pragma link C++ class Belle2::CDCTriggerTrack+; // checksum=0x589cc554, version=9
#pragma link C++ class Belle2::CDCTriggerMLP+; // checksum=0x2d137aa, version=7
#pragma link C++ class Belle2::CDCTriggerMLPData+; // checksum=0x48969a58, version=1
#pragma link C++ class Belle2::CDCTriggerMLPInput+; // checksum=0x6fd59940, version=1
#pragma link C++ class Belle2::CDCTriggerHoughCluster+; // checksum=0x8c000310, version=1
#pragma link C++ class Belle2::CDCTriggerFinderClone+; // checksum=0x6581a3cf, version=1
#pragma link C++ class Belle2::TRGCDCTSFUnpackerStore+; // checksum=0x8b9e5f2, version=1
#pragma link C++ class Belle2::TRGCDCETFUnpackerStore+; // checksum=0x3723963b, version=2

#pragma link C++ class Belle2::Bitstream<array<array<array<char, 429>, 4>, 5> >+; // checksum=0xc26a7849, version=4
#pragma link C++ class Belle2::Bitstream<array<array<array<char, TSF_TO_2D_WIDTH>, NUM_2D>, NUM_TSF> >+; // checksum=0xc3cfbe4d, version=4
#pragma link C++ class Belle2::Bitstream<array<array<bitset<TSF_TO_2D_WIDTH>, NUM_2D>,NUM_TSF> >+; // checksum=0x6af98b6d, version=4

#pragma link C++ class Belle2::TRGCDCT3DUnpackerStore+; // checksum=0x4a11bad, version=3
#pragma link C++ class Belle2::Bitstream<array<bitset<MERGER_WIDTH>, NUM_MERGER> >+; // checksum=0xc886be9, version=4
#pragma link C++ class Belle2::Bitstream<array<array<array<char, TSF_TO_2D_WIDTH>, NUM_2D>, NUM_TSF> >+; // checksum=0xc3cfbe4d, version=4
#pragma link C++ class Belle2::Bitstream<array<array<bitset<TSF_TO_2D_WIDTH>, NUM_2D>,NUM_TSF> >+; // checksum=0x6af98b6d, version=4

#pragma link C++ class Belle2::Bitstream<array<array<char, T2D_TO_3D_WIDTH>, NUM_2D> >+; // checksum=0x265dd3d5, version=4
#pragma link C++ class Belle2::Bitstream<array<array<char, NN_IN_WIDTH>, NUM_2D> >+; // checksum=0xdeda710f, version=4
#pragma link C++ class Belle2::Bitstream<array<array<char, NN_OUT_WIDTH>, NUM_2D> >+; // checksum=0x689d6f35, version=4
#pragma link C++ class Belle2::Bitstream<array<array<char, NN_WIDTH>, NUM_2D> >+; // checksum=0x9e8c810d, version=4

#endif
