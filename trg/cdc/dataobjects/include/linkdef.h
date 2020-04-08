#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;


// ROOT dictionaries will be build for all classes mentioned this way
#pragma link C++ class Belle2::CDCTriggerSegmentHit+;
#pragma link C++ class Belle2::CDCTriggerTrack+;
#pragma link C++ class Belle2::CDCTriggerHWTrack+;
#pragma link C++ class Belle2::CDCTriggerMLP+;
#pragma link C++ class Belle2::CDCTriggerMLPData+;
#pragma link C++ class Belle2::CDCTriggerMLPInput+;
#pragma link C++ class Belle2::CDCTriggerHoughCluster+;
#pragma link C++ class Belle2::CDCTriggerFinderClone+;
#pragma link C++ class Belle2::TRGCDCTSFUnpackerStore+;
#pragma link C++ class Belle2::TRGCDCETFUnpackerStore+;

#pragma link C++ class Belle2::Bitstream<array<array<array<char, 429>, 4>, 5> >+;
#pragma link C++ class Belle2::Bitstream<array<array<array<char, TSF_TO_2D_WIDTH>, NUM_2D>, NUM_TSF> >+;
#pragma link C++ class Belle2::Bitstream<array<array<bitset<TSF_TO_2D_WIDTH>, NUM_2D>,NUM_TSF> >+;

#pragma link C++ class Belle2::TRGCDCT3DUnpackerStore+;
#pragma link C++ class Belle2::Bitstream<array<bitset<MERGER_WIDTH>, NUM_MERGER> >+;
#pragma link C++ class Belle2::Bitstream<array<array<array<char, TSF_TO_2D_WIDTH>, NUM_2D>, NUM_TSF> >+;
#pragma link C++ class Belle2::Bitstream<array<array<bitset<TSF_TO_2D_WIDTH>, NUM_2D>,NUM_TSF> >+;

#pragma link C++ class Belle2::Bitstream<array<array<char, T2D_TO_3D_WIDTH>, NUM_2D> >+;
#pragma link C++ class Belle2::Bitstream<array<array<char, NN_IN_WIDTH>, NUM_2D> >+;
#pragma link C++ class Belle2::Bitstream<array<array<char, NN_OUT_WIDTH>, NUM_2D> >+;
#pragma link C++ class Belle2::Bitstream<array<array<char, NN_WIDTH>, NUM_2D> >+;

#endif
