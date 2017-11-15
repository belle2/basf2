#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;


// ROOT dictionaries will be build for all classes mentioned this way
#pragma link C++ class Belle2::CDCTriggerSegmentHit+;
#pragma link C++ class Belle2::CDCTriggerTrack+;
#pragma link C++ class Belle2::CDCTriggerMLP+;
#pragma link C++ class Belle2::CDCTriggerMLPData+;
#pragma link C++ class Belle2::CDCTriggerHoughCluster+;

#define TSF_TO_2D_HALF_WIDTH 219
#define TSF_TO_2D_WIDTH 429
#define NUM_2D 4
#define NUM_TSF 5

#pragma link C++ class Belle2::Bitstream<array<array<array<char, 429>, 4>, 5> >+;
#pragma link C++ class Belle2::Bitstream<array<array<array<char, TSF_TO_2D_WIDTH>, NUM_2D>, NUM_TSF> >+;
#pragma link C++ class Belle2::Bitstream<array<array<bitset<TSF_TO_2D_WIDTH>, NUM_2D>,NUM_TSF> >+;

#endif
