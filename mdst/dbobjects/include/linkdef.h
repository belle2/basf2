#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;


#pragma link C++ class Belle2::BeamSpot+;
#pragma link C++ class Belle2::CollisionBoostVector+;
#pragma link C++ class Belle2::CollisionInvariantMass+;
#pragma link C++ class Belle2::TRGGDLDBPrescales+;
#pragma link C++ class Belle2::TRGGDLDBFTDLBits+;
#pragma link C++ class Belle2::TRGGDLDBInputBits+;
#pragma link C++ class Belle2::TRGGDLDBBadrun+;
#pragma link C++ class Belle2::SoftwareTriggerCutBase+;
#pragma link C++ class Belle2::DBRepresentationOfSoftwareTriggerCut+;
#pragma link C++ class Belle2::SoftwareTriggerMenu+;

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

