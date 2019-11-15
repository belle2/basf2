/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi, Stefano Lacaprara                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/TagVertex.h>

using namespace Belle2;
using namespace std;


TVector3 TagVertex::getTagVertex()
{
  return m_tagVertex;
}

TMatrixDSym TagVertex::getTagVertexErrMatrix()
{
  return m_tagVertexErrMatrix;
}

float TagVertex::getTagVertexPval()
{
  return m_tagVertexPval;
}

float TagVertex::getDeltaT()
{
  return m_deltaT;
}

float TagVertex::getDeltaTErr()
{
  return m_deltaTErr;
}

TVector3 TagVertex::getMCTagVertex()
{
  return m_MCtagV;
}

int TagVertex::getMCTagBFlavor()
{
  return m_mcPDG;
}

float TagVertex::getMCDeltaT()
{
  return m_MCdeltaT;
}

int TagVertex::getFitType()
{
  return m_FitType;
}

int TagVertex::getNTracks()
{
  return m_NTracks;
}

int TagVertex::getNFitTracks()
{
  return m_NFitTracks;
}

float TagVertex::getTagVl()
{
  return m_tagVl;
}

float TagVertex::getTruthTagVl()
{
  return m_truthTagVl;
}

float TagVertex::getTagVlErr()
{
  return m_tagVlErr;
}

float TagVertex::getTagVol()
{
  return m_tagVol;
}

float TagVertex::getTruthTagVol()
{
  return m_truthTagVol;
}

float TagVertex::getTagVolErr()
{
  return m_tagVolErr;
}

float TagVertex::getTagVNDF()
{
  return m_tagVNDF;
}

float TagVertex::getTagVChi2()
{
  return m_tagVChi2;
}

float TagVertex::getTagVChi2IP()
{
  return m_tagVChi2IP;
}

double TagVertex::getVtxFitTrackPComponent(unsigned int trackIndex, unsigned int component)
{
  if (m_vtxFitTracks.size() <= trackIndex || component > 2)
    return -1111.;
  return m_vtxFitTracks.at(trackIndex) -> getMomentum()[component];
}

double TagVertex::getVtxFitTrackZ0(unsigned int trackIndex)
{
  if (m_vtxFitTracks.size() <= trackIndex)
    return -1111.;
  return m_vtxFitTracks.at(trackIndex) -> getZ0();
}

double TagVertex::getVtxFitTrackD0(unsigned int trackIndex)
{
  if (m_vtxFitTracks.size() <= trackIndex)
    return -1111.;
  return m_vtxFitTracks.at(trackIndex) -> getD0();
}

double TagVertex::getRaveWeight(unsigned int trackIndex)
{
  if (m_raveWeights.size() <= trackIndex)
    return -1111.;
  return m_raveWeights.at(trackIndex);
}


double TagVertex::getTrackDistanceToConstraint(unsigned int trackIndex)
{
  if (m_constraintType == "noConstraint" || m_vtxFitTracks.size() <= trackIndex)
    return -1111.;

  return m_distanceTools.trackToVtxDist(m_vtxFitTracks.at(trackIndex) -> getPosition(),
                                        m_vtxFitTracks.at(trackIndex) -> getMomentum(),
                                        m_constraintCenter);
}

double TagVertex::getTrackDistanceToConstraintErr(unsigned int trackIndex)
{
  if (m_constraintType == "noConstraint" || m_vtxFitTracks.size() <= trackIndex)
    return -1111.;

  TMatrixDSym trackPosCovMat(m_vtxFitTracks.at(trackIndex)->getCovariance6().GetSub(0, 2, 0, 2));

  return m_distanceTools.trackToVtxDistErr(m_vtxFitTracks.at(trackIndex) -> getPosition(),
                                           m_vtxFitTracks.at(trackIndex) -> getMomentum(),
                                           m_constraintCenter,
                                           trackPosCovMat,
                                           m_constraintCov);
}

double TagVertex::getTagVDistanceToConstraint()
{
  if (m_constraintType == "noConstraint") return -1111;

  return m_distanceTools.vtxToVtxDist(m_constraintCenter, m_tagVertex);
}

double TagVertex::getTagVDistanceToConstraintErr()
{
  if (m_constraintType == "noConstraint") return -1111;

  //To compute the uncertainty, the tag vtx uncertainty is NOT taken into account
  //The error computed is the the one used  in the chi2.
  //To change that, emptyMat has to be replaced by m_TagVertexErrMatrix
  TMatrixDSym emptyMat(3);

  return m_distanceTools.vtxToVtxDistErr(m_constraintCenter,
                                         m_tagVertex,
                                         m_constraintCov,
                                         emptyMat);
}


double TagVertex::getTrackDistanceToTagV(unsigned int trackIndex)
{
  if (m_vtxFitTracks.size() <= trackIndex)
    return -1111.;

  return m_distanceTools.trackToVtxDist(m_vtxFitTracks.at(trackIndex) -> getPosition(),
                                        m_vtxFitTracks.at(trackIndex) -> getMomentum(),
                                        m_tagVertex);
}

double TagVertex::getTrackDistanceToTagVErr(unsigned int trackIndex)
{
  if (m_vtxFitTracks.size() <= trackIndex)
    return -1111.;

  TMatrixDSym trackPosCovMat(m_vtxFitTracks.at(trackIndex)->getCovariance6().GetSub(0, 2, 0, 2));

  //To compute the uncertainty, the tag vtx uncertainty is NOT taken into account
  //The error computed is then the one in the chi2.
  //To change that, emptyMat has to be replaced by m_TagVertexErrMatrix
  TMatrixDSym emptyMat(3);

  return m_distanceTools.trackToVtxDistErr(m_vtxFitTracks.at(trackIndex) -> getPosition(),
                                           m_vtxFitTracks.at(trackIndex) -> getMomentum(),
                                           m_tagVertex,
                                           trackPosCovMat,
                                           emptyMat);
}

void TagVertex::setTagVertex(const TVector3& tagVertex)
{
  m_tagVertex = tagVertex;
}

void TagVertex::setTagVertexErrMatrix(const TMatrixDSym& TagVertexErrMatrix)
{
  m_tagVertexErrMatrix = TagVertexErrMatrix;
}

void TagVertex::setTagVertexPval(float TagVertexPval)
{
  m_tagVertexPval = TagVertexPval;
}

void TagVertex::setDeltaT(float DeltaT)
{
  m_deltaT = DeltaT;
}

void TagVertex::setDeltaTErr(float DeltaTErr)
{
  m_deltaTErr = DeltaTErr;
}

void TagVertex::setMCTagVertex(const TVector3& MCTagVertex)
{
  m_MCtagV = MCTagVertex;
}

void TagVertex::setMCTagBFlavor(int MCTagBFlavor)
{
  m_mcPDG = MCTagBFlavor;
}

void TagVertex::setMCDeltaT(float MCDeltaT)
{
  m_MCdeltaT = MCDeltaT;
}

void TagVertex::setFitType(float FitType)
{
  m_FitType = FitType;
}

void TagVertex::setNTracks(int NTracks)
{
  m_NTracks = NTracks;
}

void TagVertex::setTagVl(float TagVl)
{
  m_tagVl = TagVl;
}

void TagVertex::setTruthTagVl(float TruthTagVl)
{
  m_truthTagVl = TruthTagVl;
}

void TagVertex::setTagVlErr(float TagVlErr)
{
  m_tagVlErr = TagVlErr;
}

void TagVertex::setTagVol(float TagVol)
{
  m_tagVol = TagVol;
}

void TagVertex::setTruthTagVol(float TruthTagVol)
{
  m_truthTagVol = TruthTagVol;
}

void TagVertex::setTagVolErr(float TagVolErr)
{
  m_tagVolErr = TagVolErr;
}

void TagVertex::setTagVNDF(float TagVNDF)
{
  m_tagVNDF = TagVNDF;
}

void TagVertex::setTagVChi2(float TagVChi2)
{
  m_tagVChi2 = TagVChi2;
}

void TagVertex::setTagVChi2IP(float TagVChi2IP)
{
  m_tagVChi2IP = TagVChi2IP;
}

void TagVertex::setVertexFitTracks(std::vector<const TrackFitResult*> const& vtxFitTracks)
{
  m_vtxFitTracks = vtxFitTracks;
  m_NFitTracks = vtxFitTracks.size();
}

void TagVertex::setRaveWeights(std::vector<double> const& raveWeights)
{
  m_raveWeights = raveWeights;
}

void TagVertex::setConstraintCenter(TVector3 const& constraintCenter)
{
  m_constraintCenter = constraintCenter;
}

void TagVertex::setConstraintCov(TMatrixDSym const& constraintCov)
{
  m_constraintCov.ResizeTo(constraintCov);
  m_constraintCov = constraintCov;
}

void TagVertex::setConstraintType(std::string constraintType)
{
  m_constraintType = constraintType;
}

void  TagVertex::resetTagVertexErrorMatrix()
{
  TMatrixDSym temp(3);
  m_tagVertexErrMatrix.ResizeTo(temp);
  m_tagVertexErrMatrix = temp;
}

void  TagVertex::resetConstraintCov()
{
  TMatrixDSym temp(3);
  m_constraintCov.ResizeTo(temp);
  m_constraintCov = temp;
}
