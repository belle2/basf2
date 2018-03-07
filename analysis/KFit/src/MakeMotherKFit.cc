/**************************************************************************
 * Copyright(C) 1997 - J. Tanaka                                          *
 *                                                                        *
 * Author: J. Tanaka                                                      *
 * Contributor: J. Tanaka and                                             *
 *              conversion to Belle II structure by T. Higuchi            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/KFit/MakeMotherKFit.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::analysis;
using namespace CLHEP;

MakeMotherKFit::MakeMotherKFit(void):
  m_Vertex(Hep3Vector()),
  m_VertexError(HepSymMatrix(3, 0))
{
  m_ErrorCode = KFitError::kNoError;
  m_FlagAtDecayPoint = true;
  m_FlagVertexError = false;
  m_FlagCorrelation = false;
  m_FlagTrackVertexError = false;
  m_FlagBeforeAfter = KFitConst::kAfterFit;
  m_MagneticField = KFitConst::kDefaultMagneticField;
  m_TrackCount = 0;
  m_Charge = 0;
}


MakeMotherKFit::~MakeMotherKFit(void)
{
}


enum KFitError::ECode
MakeMotherKFit::addTrack(const KFitTrack& p) {
  m_Tracks.push_back(p);
  m_Charge += p.getCharge();
  m_TrackCount = m_Tracks.size();

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MakeMotherKFit::addTrack(const HepLorentzVector& p, const HepPoint3D& x, const HepSymMatrix& e, const double q, const int flag) {
  if (e.num_row() != (int)KFitConst::kNumber7)
  {
    m_ErrorCode = KFitError::kBadMatrixSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  return this->addTrack(KFitTrack(p, x, e, q, flag));
}


enum KFitError::ECode
MakeMotherKFit::setMagneticField(const double mf) {
  m_MagneticField = mf;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MakeMotherKFit::setVertex(const HepPoint3D& v) {
  m_Vertex = v;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MakeMotherKFit::setVertexError(const HepSymMatrix& e) {
  if (e.num_row() != 3)
  {
    m_ErrorCode = KFitError::kBadMatrixSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  m_VertexError = e;
  m_FlagVertexError = true;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MakeMotherKFit::setTrackVertexError(const HepMatrix& e) {
  if (e.num_row() != 3 || e.num_col() != (int)KFitConst::kNumber7)
  {
    m_ErrorCode = KFitError::kBadMatrixSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  m_TrackVertexError.push_back(e);
  m_FlagTrackVertexError = true;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MakeMotherKFit::setTrackZeroVertexError(void) {
  HepMatrix zero(3, KFitConst::kNumber7, 0);

  return this->setTrackVertexError(zero);
}


enum KFitError::ECode
MakeMotherKFit::setCorrelation(const HepMatrix& e) {
  if (e.num_row() != (int)KFitConst::kNumber7 || e.num_col() != (int)KFitConst::kNumber7)
  {
    m_ErrorCode = KFitError::kBadMatrixSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  m_Correlation.push_back(e);
  m_FlagCorrelation = true;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MakeMotherKFit::setZeroCorrelation(void) {
  HepMatrix zero(KFitConst::kNumber7, KFitConst::kNumber7, 0);

  return this->setCorrelation(zero);
}


enum KFitError::ECode
MakeMotherKFit::setFlagAtDecayPoint(const bool flag) {
  m_FlagAtDecayPoint = flag;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MakeMotherKFit::setFlagBeforeAfter(const int flag) {
  m_FlagBeforeAfter = flag;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MakeMotherKFit::getErrorCode(void) const {
  return m_ErrorCode;
}

const KFitTrack
MakeMotherKFit::getMother(void) const
{
  return m_Mother;
}


const HepLorentzVector
MakeMotherKFit::getMotherMomentum(void) const
{
  return m_Mother.getMomentum(KFitConst::kBeforeFit);
}


const HepPoint3D
MakeMotherKFit::getMotherPosition(void) const
{
  return m_Mother.getPosition(KFitConst::kBeforeFit);
}


const HepSymMatrix
MakeMotherKFit::getMotherError(void) const
{
  return m_Mother.getError(KFitConst::kBeforeFit);
}


enum KFitError::ECode
MakeMotherKFit::doMake(void) {
  // ...makes matrix.
  HepMatrix dMdC(KFitConst::kNumber7, KFitConst::kNumber7 * m_TrackCount + 3, 0);
  HepSymMatrix Ec(KFitConst::kNumber7 * m_TrackCount + 3, 0);


  // ...sets error matrix
  if (m_FlagCorrelation && m_Correlation.size() != static_cast<unsigned int>((m_TrackCount * m_TrackCount - m_TrackCount) / 2))
  {
    m_ErrorCode = KFitError::kBadTrackSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }
  if (m_FlagTrackVertexError && m_TrackVertexError.size() != (unsigned int)m_TrackCount)
  {
    m_ErrorCode = KFitError::kBadTrackSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }
  this->calculateError(&Ec);


  // ...makes delMdelC to calculate error matrix of mother particle.
  this->calculateDELMDELC(&dMdC);


  // ...calculates error matrix of mother particle.
  HepSymMatrix Em(Ec.similarity(dMdC));


  // ...makes mother particle
  double px = 0, py = 0, pz = 0, E = 0;
  for (int i = 0; i < m_TrackCount; i++)
  {
    double a = 0, dx = 0, dy = 0;
    if (m_FlagAtDecayPoint) {
      const double c = KFitConst::kLightSpeed; // C++ bug?
      // a = -KFitConst::kLightSpeed * m_MagneticField * m_Tracks[i].getCharge();
      a = -c * m_MagneticField * m_Tracks[i].getCharge();
      dx = m_Vertex.x() - m_Tracks[i].getPosition(m_FlagBeforeAfter).x();
      dy = m_Vertex.y() - m_Tracks[i].getPosition(m_FlagBeforeAfter).y();
    }
    px += m_Tracks[i].getMomentum(m_FlagBeforeAfter).x() - a * dy;
    py += m_Tracks[i].getMomentum(m_FlagBeforeAfter).y() + a * dx;
    pz += m_Tracks[i].getMomentum(m_FlagBeforeAfter).z();
    E  += m_Tracks[i].getMomentum(m_FlagBeforeAfter).t();
  }


  // ...momentum
  m_Mother.setMomentum(HepLorentzVector(px, py, pz, E));
  // ...position
  m_Mother.setPosition(m_Vertex);
  // ...error
  m_Mother.setError(Em);
  m_Mother.setCharge(m_Charge);


  return m_ErrorCode = KFitError::kNoError;
}


void
MakeMotherKFit::calculateError(HepSymMatrix* Ec) const
{
  // ...error matrix of tracks
  {
    int i = 0;
    for (vector<KFitTrack>::const_iterator it = m_Tracks.begin(), endIt = m_Tracks.end(); it != endIt; ++it) {
      (*Ec).sub(i * KFitConst::kNumber7 + 1, it->getError(m_FlagBeforeAfter));
      i++;
    }
  }


  // ...error matrix between tracks
  if (m_FlagCorrelation) {
    int i = 0, j = 1;
    for (vector<HepMatrix>::const_iterator it = m_Correlation.begin(), endIt = m_Correlation.end(); it != endIt; ++it) {
      const HepMatrix& hm = *it;

      for (int k = 0; k < KFitConst::kNumber7; k++) for (int l = 0; l < KFitConst::kNumber7; l++) {
          (*Ec)[k + i * KFitConst::kNumber7][l + j * KFitConst::kNumber7] = hm[k][l];
        }

      if (j != m_TrackCount - 1) {
        j++;
      } else if (i != m_TrackCount - 2) {
        i++;
        j = i + 1;
      } else {
        break;
      }
    }
  }


  // ...error of vertex
  if (m_FlagVertexError) {
    (*Ec).sub(m_TrackCount * KFitConst::kNumber7 + 1, m_VertexError);
  }


  // ...error matrix between vertex and tracks
  if (m_FlagTrackVertexError) {
    int i = 0;
    for (vector<HepMatrix>::const_iterator it = m_TrackVertexError.begin(), endIt = m_TrackVertexError.end(); it != endIt; ++it) {
      const HepMatrix& hm = *it;
      for (int j = 0; j < 3; j++) for (int k = 0; k < KFitConst::kNumber7; k++) {
          (*Ec)[j + m_TrackCount * KFitConst::kNumber7][k + i * KFitConst::kNumber7] = hm[j][k];
        }
      i++;
    }
  }
}


void
MakeMotherKFit::calculateDELMDELC(HepMatrix* dMdC) const
{
  // ...local parameters
  double sum_a = 0;

  for (int i = 0; i < m_TrackCount; i++) {
    const double c = KFitConst::kLightSpeed; // C++ bug?
    // double a = -KFitConst::kLightSpeed * m_MagneticField * m_Tracks[i].getCharge();
    double a = -c * m_MagneticField * m_Tracks[i].getCharge();
    sum_a += a;

    // ...sets "a" in dMdC.
    (*dMdC)[0][5 + i * KFitConst::kNumber7] =  a;
    (*dMdC)[1][4 + i * KFitConst::kNumber7] = -a;

    // ...sets "1" in dMdC.
    (*dMdC)[0][0 + i * KFitConst::kNumber7] = 1.;
    (*dMdC)[1][1 + i * KFitConst::kNumber7] = 1.;
    (*dMdC)[2][2 + i * KFitConst::kNumber7] = 1.;
    (*dMdC)[3][3 + i * KFitConst::kNumber7] = 1.;
  }

  // ...sets "1" in dMdC.
  (*dMdC)[4][0 + m_TrackCount * KFitConst::kNumber7] = 1.;
  (*dMdC)[5][1 + m_TrackCount * KFitConst::kNumber7] = 1.;
  (*dMdC)[6][2 + m_TrackCount * KFitConst::kNumber7] = 1.;

  // ...sets "sum_a" in dMdC.
  (*dMdC)[0][1 + m_TrackCount * KFitConst::kNumber7] = - sum_a;
  (*dMdC)[1][0 + m_TrackCount * KFitConst::kNumber7] =   sum_a;
}

