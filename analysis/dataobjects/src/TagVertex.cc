/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dataobjects/TagVertex.h>

using namespace Belle2;

static const double realNaN = std::numeric_limits<double>::quiet_NaN();
static const TVector3 vecNaN(realNaN, realNaN, realNaN);

TVector3 TagVertex::getTagVertex() const
{
  return m_tagVertex;
}

TMatrixDSym TagVertex::getTagVertexErrMatrix() const
{
  return m_tagVertexErrMatrix;
}

float TagVertex::getTagVertexPval() const
{
  return m_tagVertexPval;
}

const Particle* TagVertex::getVtxFitParticle(unsigned int trackIndex) const
{
  if (trackIndex >= m_vtxFitParticles.size())
    return 0;
  return m_vtxFitParticles.at(trackIndex);
}

const MCParticle* TagVertex::getVtxFitMCParticle(unsigned int trackIndex) const
{
  if (trackIndex >= m_vtxFitMCParticles.size())
    return 0;
  return m_vtxFitMCParticles.at(trackIndex);
}

float TagVertex::getDeltaT() const
{
  return m_deltaT;
}

float TagVertex::getDeltaTErr() const
{
  return m_deltaTErr;
}

TVector3 TagVertex::getMCTagVertex() const
{
  return m_mcTagV;
}

int TagVertex::getMCTagBFlavor() const
{
  return m_mcPDG;
}

float TagVertex::getMCDeltaTau() const
{
  return m_mcDeltaTau;
}

float TagVertex::getMCDeltaT() const
{
  return m_mcDeltaT;
}

int TagVertex::getFitType() const
{
  return m_FitType;
}

std::string TagVertex::getConstraintType() const
{
  return m_constraintType;
}

TVector3 TagVertex::getConstraintCenter() const
{
  if (m_constraintType == "noConstraint") return vecNaN;
  return m_constraintCenter;
}

TMatrixDSym TagVertex::getConstraintCov() const
{
  return m_constraintCov;
}

int TagVertex::getNTracks() const
{
  return m_NTracks;
}

int TagVertex::getNFitTracks() const
{
  return m_NFitTracks;
}

float TagVertex::getTagVl() const
{
  return m_tagVl;
}

float TagVertex::getTruthTagVl() const
{
  return m_truthTagVl;
}

float TagVertex::getTagVlErr() const
{
  return m_tagVlErr;
}

float TagVertex::getTagVol() const
{
  return m_tagVol;
}

float TagVertex::getTruthTagVol() const
{
  return m_truthTagVol;
}

float TagVertex::getTagVolErr() const
{
  return m_tagVolErr;
}

float TagVertex::getTagVNDF() const
{
  return m_tagVNDF;
}

float TagVertex::getTagVChi2() const
{
  return m_tagVChi2;
}

float TagVertex::getTagVChi2IP() const
{
  return m_tagVChi2IP;
}

TVector3 TagVertex::getVtxFitTrackPosition(unsigned int trackIndex) const
{
  if (m_vtxFitParticles.size() <= trackIndex) return vecNaN;
  return m_vtxFitParticles.at(trackIndex)->getTrackFitResult()->getPosition();
}


TVector3 TagVertex::getVtxFitTrackP(unsigned int trackIndex) const
{
  if (m_vtxFitParticles.size() <= trackIndex) return vecNaN;
  return m_vtxFitParticles.at(trackIndex)->getMomentum();
}

double TagVertex::getVtxFitTrackPComponent(unsigned int trackIndex, unsigned int component) const
{
  if (m_vtxFitParticles.size() <= trackIndex || component > 2) return realNaN;
  return m_vtxFitParticles.at(trackIndex)->getMomentum()[component];
}

double TagVertex::getVtxFitTrackZ0(unsigned int trackIndex) const
{
  if (m_vtxFitParticles.size() <= trackIndex) return realNaN;
  return m_vtxFitParticles.at(trackIndex)->getTrackFitResult()->getZ0();
}

double TagVertex::getVtxFitTrackD0(unsigned int trackIndex) const
{
  if (m_vtxFitParticles.size() <= trackIndex) return realNaN;
  return m_vtxFitParticles.at(trackIndex)->getTrackFitResult()->getD0();
}

double TagVertex::getRaveWeight(unsigned int trackIndex) const
{
  if (m_raveWeights.size() <= trackIndex) return realNaN;
  return m_raveWeights.at(trackIndex);
}

int TagVertex::getFitTruthStatus() const
{
  return m_fitTruthStatus;
}

int TagVertex::getRollBackStatus() const
{
  return m_rollbackStatus;
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

void TagVertex::setMCTagVertex(const TVector3& mcTagVertex)
{
  m_mcTagV = mcTagVertex;
}

void TagVertex::setMCTagBFlavor(int mcTagBFlavor)
{
  m_mcPDG = mcTagBFlavor;
}

void TagVertex::setMCDeltaTau(float mcDeltaTau)
{
  m_mcDeltaTau = mcDeltaTau;
}

void TagVertex::setMCDeltaT(float mcDeltaT)
{
  m_mcDeltaT = mcDeltaT;
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

void TagVertex::setVertexFitParticles(const std::vector<const Particle*>& vtxFitParticles)
{
  m_vtxFitParticles = vtxFitParticles;
  m_NFitTracks = vtxFitParticles.size();
}

void TagVertex::setVertexFitMCParticles(const std::vector<const MCParticle*>& vtxFitMCParticles)
{
  m_vtxFitMCParticles = vtxFitMCParticles;
}

void TagVertex::setRaveWeights(const std::vector<double>& raveWeights)
{
  m_raveWeights = raveWeights;
}

void TagVertex::setConstraintCenter(const TVector3& constraintCenter)
{
  m_constraintCenter = constraintCenter;
}

void TagVertex::setConstraintCov(const TMatrixDSym& constraintCov)
{
  m_constraintCov.ResizeTo(constraintCov);
  m_constraintCov = constraintCov;
}

void TagVertex::setConstraintType(const std::string& constraintType)
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

void TagVertex::setRollBackStatus(int backStatus)
{
  m_rollbackStatus = backStatus;
}
