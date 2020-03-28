/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi, Stefano Lacaprara, Thibaud Humair         *
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

const Particle* TagVertex::getVtxFitParticle(unsigned int trackIndex)
{
  if (trackIndex >= m_vtxFitParticles.size())
    return 0;
  return m_vtxFitParticles.at(trackIndex);
}

const MCParticle* TagVertex::getVtxFitMCParticle(unsigned int trackIndex)
{
  if (trackIndex >= m_vtxFitMCParticles.size())
    return 0;
  return m_vtxFitMCParticles.at(trackIndex);
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

float TagVertex::getMCDeltaTapprox()
{
  return m_MCdeltaTapprox;
}

int TagVertex::getFitType()
{
  return m_FitType;
}

string TagVertex::getConstraintType()
{
  return m_constraintType;
}

TVector3 TagVertex::getConstraintCenter()
{
  if (m_constraintType == "noConstraint")
    return TVector3(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                    std::numeric_limits<float>::quiet_NaN());
  return m_constraintCenter;
}

TMatrixDSym TagVertex::getConstraintCov()
{
  return m_constraintCov;
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

TVector3 TagVertex::getVtxFitTrackPosition(unsigned int trackIndex)
{
  if (m_vtxFitParticles.size() <= trackIndex)
    return TVector3(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                    std::numeric_limits<float>::quiet_NaN());
  return m_vtxFitParticles.at(trackIndex) -> getTrackFitResult() -> getPosition();
}


TVector3 TagVertex::getVtxFitTrackP(unsigned int trackIndex)
{
  if (m_vtxFitParticles.size() <= trackIndex)
    return TVector3(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                    std::numeric_limits<float>::quiet_NaN());
  return m_vtxFitParticles.at(trackIndex) -> getMomentum();
}

double TagVertex::getVtxFitTrackPComponent(unsigned int trackIndex, unsigned int component)
{
  if (m_vtxFitParticles.size() <= trackIndex || component > 2)
    return std::numeric_limits<float>::quiet_NaN();;
  return m_vtxFitParticles.at(trackIndex) -> getMomentum()[component];
}

double TagVertex::getVtxFitTrackZ0(unsigned int trackIndex)
{
  if (m_vtxFitParticles.size() <= trackIndex)
    return std::numeric_limits<float>::quiet_NaN();;
  return m_vtxFitParticles.at(trackIndex) -> getTrackFitResult() -> getZ0();
}

double TagVertex::getVtxFitTrackD0(unsigned int trackIndex)
{
  if (m_vtxFitParticles.size() <= trackIndex)
    return std::numeric_limits<float>::quiet_NaN();;
  return m_vtxFitParticles.at(trackIndex) -> getTrackFitResult() -> getD0();
}

double TagVertex::getRaveWeight(unsigned int trackIndex)
{
  if (m_raveWeights.size() <= trackIndex)
    return std::numeric_limits<float>::quiet_NaN();;
  return m_raveWeights.at(trackIndex);
}

int TagVertex::getFitTruthStatus()
{
  return m_fitTruthStatus;
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

void TagVertex::setMCDeltaTapprox(float MCDeltaTapprox)
{
  m_MCdeltaTapprox = MCDeltaTapprox;
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

void TagVertex::setVertexFitParticles(std::vector<const Particle*> const& vtxFitParticles)
{
  m_vtxFitParticles = vtxFitParticles;
  m_NFitTracks = vtxFitParticles.size();
}

void TagVertex::setVertexFitMCParticles(std::vector<const MCParticle*> const& vtxFitMCParticles)
{
  m_vtxFitMCParticles = vtxFitMCParticles;
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

void TagVertex::setConstraintType(std::string const& constraintType)
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

void TagVertex::setFitTruthStatus(int truthStatus)
{
  m_fitTruthStatus = truthStatus;
}
