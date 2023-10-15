
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;


// ROOT dictionaries will be build for all classes mentioned this way
#pragma link C++ class bitset <639>+; // checksum=0x88f24d, version=2
#pragma link C++ class bitset <256>+; // checksum=0x88f1ea, version=2
#pragma link C++ class Belle2::CDCTriggerSegmentHit+; // checksum=0x89ed4679, version=4
#pragma link C++ class Belle2::CDCTriggerTrack+; // checksum=0xa5ae9350, version=13
#pragma link C++ class Belle2::CDCTriggerMLP+; // checksum=0x2a3c1c8d, version=10
#pragma link C++ class Belle2::CDCTriggerMLPData+; // checksum=0x62b29f0b, version=3
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











#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<unsigned short> nNodes"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_nNodes"                                                            \
  code="{for (const auto& w : onfile.nNodes)                              \
           m_nNodes.emplace_back(w);}"



#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> weights"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_weights"                                                            \
  code="{for (const auto& w : onfile.weights)                              \
           m_weights.emplace_back(w);    \
        }"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="bool trained"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_trained"                                                            \
  code="{m_trained = onfile.trained;}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="unsigned short targetVars"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_targetVars"                                                            \
  code="{m_targetVars = onfile.targetVars;}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> outputScale"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_outputScale"                                                            \
  code="{for (const auto& w : onfile.outputScale)                              \
           m_outputScale.emplace_back(w);}"
//####################################################################################
#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> phiRange"                                      \
  version="[-7]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_phiRangeUse"                                                            \
  code="{for (const auto& w : onfile.phiRange)                              \
           m_phiRangeUse.emplace_back(w);}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> invptRange"                                      \
  version="[-7]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_invptRangeUse"                                                            \
  code="{for (const auto& w : onfile.invptRange)                              \
           m_invptRangeUse.emplace_back(w);}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> thetaRange"                                      \
  version="[-7]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_thetaRangeUse"                                                            \
  code="{for (const auto& w : onfile.thetaRange)                              \
           m_thetaRangeUse.emplace_back(w);}"


#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> thetaRange"                                      \
  version="[-7]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_thetaRangeTrain"                                                            \
  code="{for (const auto& w : onfile.thetaRange)                              \
           m_thetaRangeTrain.emplace_back(w);}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> phiRange"                                      \
  version="[-7]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_phiRangeTrain"                                                            \
  code="{for (const auto& w : onfile.phiRange)                              \
           m_phiRangeTrain.emplace_back(w);}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> invptRange"                                      \
  version="[-7]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_invptRangeTrain"                                                            \
  code="{for (const auto& w : onfile.invptRange)                              \
           m_invptRangeTrain.emplace_back(w);}"

//##################################################################
#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> phiRangeUse"                                      \
  version="[8-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_phiRangeUse"                                                            \
  code="{for (const auto& w : onfile.phiRangeUse)                              \
           m_phiRangeUse.emplace_back(w);}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> invptRangeUse"                                      \
  version="[8-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_invptRangeUse"                                                            \
  code="{for (const auto& w : onfile.invptRangeUse)                              \
           m_invptRangeUse.emplace_back(w);}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> thetaRangeUse"                                      \
  version="[8-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_thetaRangeUse"                                                            \
  code="{for (const auto& w : onfile.thetaRangeUse)                              \
           m_thetaRangeUse.emplace_back(w);}"


#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> thetaRangeTrain"                                      \
  version="[8-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_thetaRangeTrain"                                                            \
  code="{for (const auto& w : onfile.thetaRangeTrain)                              \
           m_thetaRangeTrain.emplace_back(w);}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> phiRangeTrain"                                      \
  version="[8-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_phiRangeTrain"                                                            \
  code="{for (const auto& w : onfile.phiRangeTrain)                              \
           m_phiRangeTrain.emplace_back(w);}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> invptRangeTrain"                                      \
  version="[8-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_invptRangeTrain"                                                            \
  code="{for (const auto& w : onfile.invptRangeTrain)                              \
           m_invptRangeTrain.emplace_back(w);}"


//#######################################################################
#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="unsigned short maxHitsPerSL"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_maxHitsPerSL"                                                            \
  code="{m_maxHitsPerSL = onfile.maxHitsPerSL;}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="unsigned long SLpattern"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_SLpattern"                                                            \
  code="{m_SLpattern = onfile.SLpattern;}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="unsigned long SLpatternMask"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_SLpatternMask"                                                            \
  code="{m_SLpatternMask = onfile.SLpatternMask;}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="unsigned short tMax"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_tMax"                                                            \
  code="{m_tMax = onfile.tMax;}"

#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::vector<float> relevantID"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_relevantID"                                                            \
  code="{for (const auto& w : onfile.relevantID) m_relevantID.emplace_back(w);}"


#pragma read                                                                \
  sourceClass="Belle2::CDCTriggerMLP"                              \
  source="std::string et_option"                                      \
  version="[-10]"                                                             \
  targetClass="Belle2::CDCTriggerMLP"                              \
  target="m_et_option"                                                            \
  code="{m_et_option = onfile.et_option;}"




#endif
