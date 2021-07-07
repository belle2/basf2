#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;


#pragma link C++ class Belle2::BeamSpot+; // checksum=0x6f933bf2, version=2
#pragma link C++ class Belle2::CollisionBoostVector+; // checksum=0x1721913a, version=1
#pragma link C++ class Belle2::CollisionInvariantMass+; // checksum=0xeb4ae2fa, version=1
#pragma link C++ class Belle2::TRGGDLDBPrescales+; // checksum=0x29260a69, version=1
#pragma link C++ class Belle2::TRGGDLDBFTDLBits+; // checksum=0x701ea2e0, version=1
#pragma link C++ class Belle2::TRGGDLDBInputBits+; // checksum=0x280e74a6, version=1
#pragma link C++ class Belle2::TRGGDLDBBadrun+; // checksum=0x7ca49175, version=1
#pragma link C++ class Belle2::SoftwareTriggerCutBase+; // checksum=0x8470ca2a, version=-1
#pragma link C++ class Belle2::DBRepresentationOfSoftwareTriggerCut+; // checksum=0xaaf47daf, version=2
#pragma link C++ class Belle2::SoftwareTriggerMenu+; // checksum=0xd05aa8b, version=1
#pragma link C++ class Belle2::TTDOffsets+; // checksum=0x78b2c7ea, version=1

#endif


// ----------------------------------------------------------------------------
// BeamSpot
// As of version 2, the names of the data members are changed to better describe the content:
//classDef = 1 -> classDef = 2
// TVector3 m_position ->  TVector3 m_IPPosition
// TMatrixDSym m_positionError -> TMatrixDSym m_IPPositionCovMatrix,
// TMatrixDSym m_size -> TMatrixDSym m_sizeCovMatrix
//
#pragma read sourceClass="Belle2::BeamSpot" version="[-1]" \
  source="TVector3 m_position" \
  targetClass="Belle2::BeamSpot" target="m_IPPosition" \
  code="{ \
m_IPPosition = onfile.m_position;\
}"

#pragma read sourceClass="Belle2::BeamSpot" version="[-1]" \
  source="TMatrixDSym m_positionError" \
  targetClass="Belle2::BeamSpot" target="m_IPPositionCovMatrix" \
  code="{ \
m_IPPositionCovMatrix = onfile.m_positionError;	\
}"

#pragma read sourceClass="Belle2::BeamSpot" version="[-1]" \
  source="TMatrixDSym m_size" \
  targetClass="Belle2::BeamSpot" target="m_sizeCovMatrix" \
  code="{ \
m_sizeCovMatrix = onfile.m_size;	\
}"

