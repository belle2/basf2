/**************************************************************************
 * Copyright(C) 1997 - J. Tanaka                                          *
 *                                                                        *
 * Author: J. Tanaka                                                      *
 * Contributor: J. Tanaka and                                             *
 *              conversion to Belle II structure by T. Higuchi            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <TMatrixFSym.h>

#include <analysis/KFit/KFitBase.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::analysis;
using namespace CLHEP;

KFitBase::KFitBase(void)
{
  m_ErrorCode = KFitError::kNoError;
  m_FlagFitted = false;
  m_FlagCorrelation = false;
  m_FlagOverIteration = false;
  m_MagneticField = KFitConst::kDefaultMagneticField;
  m_NDF = 0;
  m_CHIsq = -1;
  m_NecessaryTrackCount = -1;
  m_TrackCount = 0;
}


KFitBase::~KFitBase(void)
{
}


enum KFitError::ECode
KFitBase::addTrack(const KFitTrack& p) {
  m_Tracks.push_back(p);
  m_TrackCount = m_Tracks.size();

  return m_ErrorCode = KFitError::kNoError;
}

enum KFitError::ECode
KFitBase::addTrack(const HepLorentzVector& p, const HepPoint3D& x, const HepSymMatrix& e, const double q) {
  if (e.num_row() != KFitConst::kNumber7)
  {
    m_ErrorCode = KFitError::kBadMatrixSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  return this->addTrack(KFitTrack(p, x, e, q));
}

enum KFitError::ECode KFitBase::addParticle(const Particle* particle)
{
  CLHEP::HepLorentzVector mom(particle->getPx(),
                              particle->getPy(),
                              particle->getPz(),
                              particle->getEnergy());
  HepPoint3D pos(particle->getX(), particle->getY(), particle->getZ());
  CLHEP::HepSymMatrix covMatrix(7);
  TMatrixFSym errMatrix = particle->getMomentumVertexErrorMatrix();
  for (int i = 0; i < 7; i++) {
    for (int j = i; j < 7; j++) {
      covMatrix[i][j] = errMatrix[i][j];
    }
  }
  return addTrack(mom, pos, covMatrix, particle->getCharge());
}


enum KFitError::ECode
KFitBase::setCorrelation(const HepMatrix& e) {
  if (e.num_row() != KFitConst::kNumber7)
  {
    m_ErrorCode = KFitError::kBadMatrixSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }
  m_BeforeCorrelation.push_back(e);
  m_FlagCorrelation = true;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
KFitBase::setZeroCorrelation(void) {
  HepMatrix zero(KFitConst::kNumber7, KFitConst::kNumber7, 0);

  return this->setCorrelation(zero);
}


enum KFitError::ECode
KFitBase::setMagneticField(const double mf) {
  m_MagneticField = mf;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
KFitBase::getErrorCode(void) const {
  return m_ErrorCode;
}


int
KFitBase::getTrackCount(void) const
{
  return m_TrackCount;
}


int
KFitBase::getNDF(void) const
{
  return m_NDF;
}


double
KFitBase::getCHIsq(void) const
{
  return m_CHIsq;
}


double
KFitBase::getMagneticField(void) const
{
  return m_MagneticField;
}


double
KFitBase::getTrackCHIsq(const int id) const
{
  if (!isFitted()) return -1.;
  if (!isTrackIDInRange(id)) return -1.;

  HepMatrix da(m_Tracks[id].getFitParameter(KFitConst::kBeforeFit) - m_Tracks[id].getFitParameter(KFitConst::kAfterFit));
  int err_inverse = 0;
  const double chisq = (da.T() * (m_Tracks[id].getFitError(KFitConst::kBeforeFit).inverse(err_inverse)) * da)[0][0];

  if (err_inverse) {
    KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kCannotGetMatrixInverse);
    return -1.;
  }

  return chisq;
}


const HepLorentzVector
KFitBase::getTrackMomentum(const int id) const
{
  if (!isTrackIDInRange(id)) return HepLorentzVector();
  return m_Tracks[id].getMomentum();
}


const HepPoint3D
KFitBase::getTrackPosition(const int id) const
{
  if (!isTrackIDInRange(id)) return HepPoint3D();
  return m_Tracks[id].getPosition();
}


const HepSymMatrix
KFitBase::getTrackError(const int id) const
{
  if (!isTrackIDInRange(id)) return HepSymMatrix(KFitConst::kNumber7, 0);
  return m_Tracks[id].getError();
}


const KFitTrack
KFitBase::getTrack(const int id) const
{
  if (!isTrackIDInRange(id)) return KFitTrack();
  return m_Tracks[id];
}


const HepMatrix
KFitBase::getCorrelation(const int id1, const int id2, const int flag) const
{
  if (flag == KFitConst::kAfterFit && !isFitted()) return HepMatrix(KFitConst::kNumber7, KFitConst::kNumber7, 0);
  if (!isTrackIDInRange(id1)) return HepMatrix(KFitConst::kNumber7, KFitConst::kNumber7, 0);
  if (!isTrackIDInRange(id2)) return HepMatrix(KFitConst::kNumber7, KFitConst::kNumber7, 0);

  switch (flag) {
    case KFitConst::kAfterFit:
      return makeError1(
               getTrackMomentum(id1),
               getTrackMomentum(id2),
               m_V_al_1.sub(KFitConst::kNumber6 * id1 + 1, KFitConst::kNumber6 * (id1 + 1), KFitConst::kNumber6 * id2 + 1,
                            KFitConst::kNumber6 * (id2 + 1))
             );

    default:
      if (id1 == id2) {

        return static_cast<HepMatrix>(m_Tracks[id1].getError(KFitConst::kBeforeFit));

      } else {
        const int idx1 = id1 < id2 ? id1 : id2, idx2 = id1 < id2 ? id2 : id1;

        int index = 0;

        for (int i = 0; i < idx1; i++) index += m_TrackCount - 1 - i;
        index -= idx1 + 1;
        index += idx2;
        if (id1 == idx1)
          return m_BeforeCorrelation[index + idx2];
        else
          return m_BeforeCorrelation[index + idx2].T();
      }
  }
}


const HepSymMatrix
KFitBase::makeError1(const HepLorentzVector& p, const HepMatrix& e) const
{
  // self track
  // Error(6x6,e) ==> Error(7x7,output(hsm)) using Momentum(p).

  if (!isNonZeroEnergy(p)) return HepSymMatrix(KFitConst::kNumber7, 0);

  HepSymMatrix hsm(KFitConst::kNumber7, 0);

  for (int i = 0; i < 3; i++) for (int j = i; j < 3; j++) {
      hsm[i][j]     = e[i][j];
      hsm[4 + i][4 + j] = e[3 + i][3 + j];
    }
  for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
      hsm[i][4 + j] = e[i][3 + j];
    }

  const double invE = 1 / p.t();
  hsm[0][3] = (p.x() * hsm[0][0] + p.y() * hsm[0][1] + p.z() * hsm[0][2]) * invE;
  hsm[1][3] = (p.x() * hsm[0][1] + p.y() * hsm[1][1] + p.z() * hsm[1][2]) * invE;
  hsm[2][3] = (p.x() * hsm[0][2] + p.y() * hsm[1][2] + p.z() * hsm[2][2]) * invE;
  hsm[3][3] = (p.x() * p.x() * hsm[0][0] + p.y() * p.y() * hsm[1][1] + p.z() * p.z() * hsm[2][2]
               + 2.0 * p.x() * p.y() * hsm[0][1]
               + 2.0 * p.x() * p.z() * hsm[0][2]
               + 2.0 * p.y() * p.z() * hsm[1][2]) * invE * invE;
  hsm[3][4] = (p.x() * hsm[0][4] + p.y() * hsm[1][4] + p.z() * hsm[2][4]) * invE;
  hsm[3][5] = (p.x() * hsm[0][5] + p.y() * hsm[1][5] + p.z() * hsm[2][5]) * invE;
  hsm[3][6] = (p.x() * hsm[0][6] + p.y() * hsm[1][6] + p.z() * hsm[2][6]) * invE;

  return hsm;
}


const HepMatrix
KFitBase::makeError1(const HepLorentzVector& p1, const HepLorentzVector& p2, const HepMatrix& e) const
{
  // track and track
  // Error(6x6,e) ==> Error(7x7,output(hm)) using Momentum(p1&p2).

  if (!isNonZeroEnergy(p1)) return HepSymMatrix(KFitConst::kNumber7, 0);
  if (!isNonZeroEnergy(p2)) return HepSymMatrix(KFitConst::kNumber7, 0);

  HepMatrix hm(KFitConst::kNumber7, KFitConst::kNumber7, 0);

  for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
      hm[i][j]     = e[i][j];
      hm[4 + i][4 + j] = e[3 + i][3 + j];
      hm[4 + i][j]   = e[3 + i][j];
      hm[i][4 + j]   = e[i][3 + j];
    }

  const double invE1 = 1 / p1.t();
  const double invE2 = 1 / p2.t();
  hm[0][3] = (p2.x() * hm[0][0] + p2.y() * hm[0][1] + p2.z() * hm[0][2]) * invE2;
  hm[1][3] = (p2.x() * hm[1][0] + p2.y() * hm[1][1] + p2.z() * hm[1][2]) * invE2;
  hm[2][3] = (p2.x() * hm[2][0] + p2.y() * hm[2][1] + p2.z() * hm[2][2]) * invE2;
  hm[4][3] = (p2.x() * hm[4][0] + p2.y() * hm[4][1] + p2.z() * hm[4][2]) * invE2;
  hm[5][3] = (p2.x() * hm[5][0] + p2.y() * hm[5][1] + p2.z() * hm[5][2]) * invE2;
  hm[6][3] = (p2.x() * hm[6][0] + p2.y() * hm[6][1] + p2.z() * hm[6][2]) * invE2;
  hm[3][3] = (p1.x() * p2.x() * hm[0][0] + p1.y() * p2.y() * hm[1][1] + p1.z() * p2.z() * hm[2][2] +
              p1.x() * p2.y() * hm[0][1] + p2.x() * p1.y() * hm[1][0] +
              p1.x() * p2.z() * hm[0][2] + p2.x() * p1.z() * hm[2][0] +
              p1.y() * p2.z() * hm[1][2] + p2.y() * p1.z() * hm[2][1]) * invE1 * invE2;
  hm[3][0] = (p1.x() * hm[0][0] + p1.y() * hm[1][0] + p1.z() * hm[2][0]) * invE1;
  hm[3][1] = (p1.x() * hm[0][1] + p1.y() * hm[1][1] + p1.z() * hm[2][1]) * invE1;
  hm[3][2] = (p1.x() * hm[0][2] + p1.y() * hm[1][2] + p1.z() * hm[2][2]) * invE1;
  hm[3][4] = (p1.x() * hm[0][4] + p1.y() * hm[1][4] + p1.z() * hm[2][4]) * invE1;
  hm[3][5] = (p1.x() * hm[0][5] + p1.y() * hm[1][5] + p1.z() * hm[2][5]) * invE1;
  hm[3][6] = (p1.x() * hm[0][6] + p1.y() * hm[1][6] + p1.z() * hm[2][6]) * invE1;

  return hm;
}


const HepMatrix
KFitBase::makeError2(const HepLorentzVector& p, const HepMatrix& e) const
{
  // vertex and track
  // Error(3x6,e) ==> Error(3x7,output(hm)) using Momentum(p).

  if (!isNonZeroEnergy(p)) return HepSymMatrix(KFitConst::kNumber7, 0);

  HepMatrix hm(3, KFitConst::kNumber7, 0);

  for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
      hm[i][j]   = e[i][j];
      hm[i][4 + j] = e[i][3 + j];
    }

  const double invE = 1 / p.t();
  hm[0][3] = (p.x() * hm[0][0] + p.y() * hm[0][1] + p.z() * hm[0][2]) * invE;
  hm[1][3] = (p.x() * hm[1][0] + p.y() * hm[1][1] + p.z() * hm[1][2]) * invE;
  hm[2][3] = (p.x() * hm[2][0] + p.y() * hm[2][1] + p.z() * hm[2][2]) * invE;

  return hm;
}


const HepSymMatrix
KFitBase::makeError3(const HepLorentzVector& p, const HepMatrix& e, const bool is_fix_mass) const
{
  // self track
  // Error(7x7,e) ==> Error(7x7,output(hsm)) using Momentum(p).
  // is_fix_mass = 1 : Energy term is recalculated from the other parameters.
  // is_fix_mass = 0 : hsm = e.

  if (!isNonZeroEnergy(p)) return HepSymMatrix(KFitConst::kNumber7, 0);

  if (!is_fix_mass) {
    HepSymMatrix hsm(KFitConst::kNumber7, 0);
    for (int i = 0; i < 7; i++) for (int j = i; j < 7; j++) {
        hsm[i][j] = e[i][j];
      }
    return hsm;
  }

  HepSymMatrix hsm(KFitConst::kNumber7, 0);

  for (int i = 0; i < 7; i++) {
    if (i != 3)
      for (int j = i; j < 7; j++) hsm[i][j] = e[i][j];
  }

  double invE = 1 / p.t();
  hsm[0][3] = (p.x() * hsm[0][0] + p.y() * hsm[0][1] + p.z() * hsm[0][2]) * invE;
  hsm[1][3] = (p.x() * hsm[0][1] + p.y() * hsm[1][1] + p.z() * hsm[1][2]) * invE;
  hsm[2][3] = (p.x() * hsm[0][2] + p.y() * hsm[1][2] + p.z() * hsm[2][2]) * invE;
  hsm[3][3] = (p.x() * p.x() * hsm[0][0] + p.y() * p.y() * hsm[1][1] + p.z() * p.z() * hsm[2][2]
               + 2.0 * p.x() * p.y() * hsm[0][1]
               + 2.0 * p.x() * p.z() * hsm[0][2]
               + 2.0 * p.y() * p.z() * hsm[1][2]) * invE * invE;
  hsm[3][4] = (p.x() * hsm[0][4] + p.y() * hsm[1][4] + p.z() * hsm[2][4]) * invE;
  hsm[3][5] = (p.x() * hsm[0][5] + p.y() * hsm[1][5] + p.z() * hsm[2][5]) * invE;
  hsm[3][6] = (p.x() * hsm[0][6] + p.y() * hsm[1][6] + p.z() * hsm[2][6]) * invE;

  return hsm;
}


const HepMatrix
KFitBase::makeError3(const HepLorentzVector& p1, const HepLorentzVector& p2, const HepMatrix& e, const bool is_fix_mass1,
                     const bool is_fix_mass2) const
{
  // track and track
  // Error(7x7,e) ==> Error(7x7,output(hm)) using Momentum(p1&p2).
  // is_fix_mass = 1 : Energy term is recalculated from the other parameters.
  // is_fix_mass = 0 : not.

  if (is_fix_mass1 && is_fix_mass2) {
    if (!isNonZeroEnergy(p1)) return HepSymMatrix(KFitConst::kNumber7, 0);
    if (!isNonZeroEnergy(p2)) return HepSymMatrix(KFitConst::kNumber7, 0);

    HepMatrix hm(e);

    const double invE1 = 1 / p1.t();
    const double invE2 = 1 / p2.t();
    hm[0][3] = (p2.x() * hm[0][0] + p2.y() * hm[0][1] + p2.z() * hm[0][2]) * invE2;
    hm[1][3] = (p2.x() * hm[1][0] + p2.y() * hm[1][1] + p2.z() * hm[1][2]) * invE2;
    hm[2][3] = (p2.x() * hm[2][0] + p2.y() * hm[2][1] + p2.z() * hm[2][2]) * invE2;
    hm[4][3] = (p2.x() * hm[4][0] + p2.y() * hm[4][1] + p2.z() * hm[4][2]) * invE2;
    hm[5][3] = (p2.x() * hm[5][0] + p2.y() * hm[5][1] + p2.z() * hm[5][2]) * invE2;
    hm[6][3] = (p2.x() * hm[6][0] + p2.y() * hm[6][1] + p2.z() * hm[6][2]) * invE2;
    hm[3][0] = (p1.x() * hm[0][0] + p1.y() * hm[1][0] + p1.z() * hm[2][0]) * invE1;
    hm[3][1] = (p1.x() * hm[0][1] + p1.y() * hm[1][1] + p1.z() * hm[2][1]) * invE1;
    hm[3][2] = (p1.x() * hm[0][2] + p1.y() * hm[1][2] + p1.z() * hm[2][2]) * invE1;
    hm[3][3] = (p1.x() * p2.x() * hm[0][0] + p1.y() * p2.y() * hm[1][1] + p1.z() * p2.z() * hm[2][2] +
                p1.x() * p2.y() * hm[0][1] + p2.x() * p1.y() * hm[1][0] +
                p1.x() * p2.z() * hm[0][2] + p2.x() * p1.z() * hm[2][0] +
                p1.y() * p2.z() * hm[1][2] + p2.y() * p1.z() * hm[2][1]) * invE1 * invE2;
    hm[3][4] = (p1.x() * hm[0][4] + p1.y() * hm[1][4] + p1.z() * hm[2][4]) * invE1;
    hm[3][5] = (p1.x() * hm[0][5] + p1.y() * hm[1][5] + p1.z() * hm[2][5]) * invE1;
    hm[3][6] = (p1.x() * hm[0][6] + p1.y() * hm[1][6] + p1.z() * hm[2][6]) * invE1;

    return hm;
  }


  if (is_fix_mass1 && !is_fix_mass2) {
    if (!isNonZeroEnergy(p1)) return HepSymMatrix(KFitConst::kNumber7, 0);

    HepMatrix hm(e);

    const double invE1 = 1 / p1.t();
    hm[3][0] = (p1.x() * hm[0][0] + p1.y() * hm[1][0] + p1.z() * hm[2][0]) * invE1;
    hm[3][1] = (p1.x() * hm[0][1] + p1.y() * hm[1][1] + p1.z() * hm[2][1]) * invE1;
    hm[3][2] = (p1.x() * hm[0][2] + p1.y() * hm[1][2] + p1.z() * hm[2][2]) * invE1;
    hm[3][3] = (p1.x() * hm[0][3] + p1.y() * hm[1][3] + p1.z() * hm[2][3]) * invE1;
    hm[3][4] = (p1.x() * hm[0][4] + p1.y() * hm[1][4] + p1.z() * hm[2][4]) * invE1;
    hm[3][5] = (p1.x() * hm[0][5] + p1.y() * hm[1][5] + p1.z() * hm[2][5]) * invE1;
    hm[3][6] = (p1.x() * hm[0][6] + p1.y() * hm[1][6] + p1.z() * hm[2][6]) * invE1;

    return hm;
  }


  if (!is_fix_mass1 &&  is_fix_mass2) {
    if (!isNonZeroEnergy(p2)) return HepSymMatrix(KFitConst::kNumber7, 0);

    HepMatrix hm(e);

    const double invE2 = 1 / p2.t();
    hm[0][3] = (p2.x() * hm[0][0] + p2.y() * hm[0][1] + p2.z() * hm[0][2]) * invE2;
    hm[1][3] = (p2.x() * hm[1][0] + p2.y() * hm[1][1] + p2.z() * hm[1][2]) * invE2;
    hm[2][3] = (p2.x() * hm[2][0] + p2.y() * hm[2][1] + p2.z() * hm[2][2]) * invE2;
    hm[3][3] = (p2.x() * hm[3][0] + p2.y() * hm[3][1] + p2.z() * hm[3][2]) * invE2;
    hm[4][3] = (p2.x() * hm[4][0] + p2.y() * hm[4][1] + p2.z() * hm[4][2]) * invE2;
    hm[5][3] = (p2.x() * hm[5][0] + p2.y() * hm[5][1] + p2.z() * hm[5][2]) * invE2;
    hm[6][3] = (p2.x() * hm[6][0] + p2.y() * hm[6][1] + p2.z() * hm[6][2]) * invE2;

    return hm;
  }

  return e;
}


const HepMatrix
KFitBase::makeError4(const HepLorentzVector& p, const HepMatrix& e) const
{
  // vertex and track
  // Error(3x7,e) ==> Error(3x7,output(hm)) using Momentum(p).
  // Energy term is recalculated from the other parameters.

  if (!isNonZeroEnergy(p)) return HepSymMatrix(KFitConst::kNumber7, 0);

  HepMatrix hm(e);

  const double invE = 1 / p.t();
  hm[0][3] = (p.x() * hm[0][0] + p.y() * hm[0][1] + p.z() * hm[0][2]) * invE;
  hm[1][3] = (p.x() * hm[1][0] + p.y() * hm[1][1] + p.z() * hm[1][2]) * invE;
  hm[2][3] = (p.x() * hm[2][0] + p.y() * hm[2][1] + p.z() * hm[2][2]) * invE;

  return hm;
}


enum KFitError::ECode
KFitBase::prepareCorrelation(void) {
  if (m_BeforeCorrelation.size() != (double)m_TrackCount * ((double)m_TrackCount - 1)*.5)
  {
    m_ErrorCode = KFitError::kBadCorrelationSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  HepMatrix tmp_hm(KFitConst::kNumber6, KFitConst::kNumber6, 0);
  int row = 0, col = 0;

  for (vector<HepMatrix>::const_iterator it = m_BeforeCorrelation.begin(), endIt = m_BeforeCorrelation.end(); it != endIt; ++it)
  {
    const HepMatrix& hm = *it;

    row++;
    if (row == m_TrackCount) {
      col++;
      row = col + 1;
    }

    // 7x7 --> 6x6
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        tmp_hm[i][j]     = hm[i][j];
        tmp_hm[3 + i][3 + j] = hm[4 + i][4 + j];
        tmp_hm[3 + i][j]   = hm[4 + i][j];
        tmp_hm[i][3 + j]   = hm[i][4 + j];
      }

    int ii = 0, jj = 0;
    for (int i = KFitConst::kNumber6 * row; i < KFitConst::kNumber6 * (row + 1); i++) {
      for (int j = KFitConst::kNumber6 * col; j < KFitConst::kNumber6 * (col + 1); j++) {
        m_V_al_0[i][j] = tmp_hm[ii][jj];
        jj++;
      }
      jj = 0;
      ii++;
    }
  }

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
KFitBase::doFit1(void) {
  if (m_ErrorCode != KFitError::kNoError) return m_ErrorCode;

  if (m_TrackCount < m_NecessaryTrackCount)
  {
    m_ErrorCode = KFitError::kBadTrackSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  if (prepareInputMatrix() != KFitError::kNoError) return m_ErrorCode;
  if (calculateNDF() != KFitError::kNoError)       return m_ErrorCode;


  double chisq = 0;
  double tmp_chisq = KFitConst::kInitialCHIsq;
  int err_inverse = 0;

  HepMatrix tmp_al_1(m_al_1);
  HepMatrix tmp_V_al_1(m_V_al_1);

  m_al_a = m_al_0;
  HepMatrix tmp_al_a(m_al_a);


  for (int i = 0; i < KFitConst::kMaxIterationCount; i++)
  {
    if (makeCoreMatrix() != KFitError::kNoError) return m_ErrorCode;

    m_V_D = (m_V_al_0.similarity(m_D)).inverse(err_inverse);
    if (err_inverse != 0) {
      m_ErrorCode = KFitError::kCannotGetMatrixInverse;
      return m_ErrorCode;
    }

    m_lam    = m_V_D * (m_D * (m_al_0 - m_al_1) + m_d);
    chisq    = ((m_lam.T()) * (m_D * (m_al_0 - m_al_1) + m_d))(1, 1);
    m_al_1   = m_al_0 - m_V_al_0 * (m_D.T()) * m_lam;
    m_V_al_1 = m_V_al_0 - m_V_al_0 * (m_D.T()) * m_V_D * m_D * m_V_al_0;

    if (tmp_chisq <= chisq) {
      if (i == 0) {
        m_ErrorCode = KFitError::kBadInitialCHIsq;
        return m_ErrorCode;
      } else {
        chisq    = tmp_chisq;
        m_al_1   = tmp_al_1;
        m_al_a   = tmp_al_a;
        m_V_al_1 = tmp_V_al_1;
        break;
      }
    } else {
      tmp_chisq  = chisq;
      tmp_al_a   = tmp_al_1;
      tmp_al_1   = m_al_1;
      tmp_V_al_1 = m_V_al_1;
      if (i == KFitConst::kMaxIterationCount - 1) {
        m_al_a = tmp_al_1;
        m_FlagOverIteration = true;
      }
    }
  }

  if (m_ErrorCode != KFitError::kNoError) return m_ErrorCode;

  if (prepareOutputMatrix() != KFitError::kNoError) return m_ErrorCode;

  m_CHIsq = chisq;

  m_FlagFitted = true;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
KFitBase::doFit2(void) {
  if (m_ErrorCode != KFitError::kNoError) return m_ErrorCode;

  if (m_TrackCount < m_NecessaryTrackCount)
  {
    m_ErrorCode = KFitError::kBadTrackSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  if (prepareInputMatrix() != KFitError::kNoError) return m_ErrorCode;
  if (calculateNDF() != KFitError::kNoError)       return m_ErrorCode;


  double chisq = 0;
  double tmp_chisq = KFitConst::kInitialCHIsq;
  double tmp2_chisq = KFitConst::kInitialCHIsq;
  int err_inverse = 0;

  m_al_a = m_al_0;
  HepMatrix tmp_al_a(m_al_a);

  HepMatrix tmp_D(m_D), tmp_E(m_E);
  HepMatrix tmp_V_D(m_V_D), tmp_V_E(m_V_E);
  HepMatrix tmp_lam0(m_lam0), tmp_v_a(m_v_a);

  HepMatrix tmp2_D(m_D), tmp2_E(m_E);
  HepMatrix tmp2_V_D(m_V_D), tmp2_V_E(m_V_E);
  HepMatrix tmp2_lam0(m_lam0), tmp2_v_a(m_v_a), tmp2_v(m_v_a);


  for (int j = 0; j < KFitConst::kMaxIterationCount; j++)   // j'th loop start
  {

    tmp_chisq = KFitConst::kInitialCHIsq;

    for (int i = 0; i < KFitConst::kMaxIterationCount; i++) { // i'th loop start

      if (prepareInputSubMatrix() != KFitError::kNoError) return m_ErrorCode;
      if (makeCoreMatrix() != KFitError::kNoError)        return m_ErrorCode;

      m_V_D = (m_V_al_0.similarity(m_D)).inverse(err_inverse);
      if (err_inverse) {
        m_ErrorCode = KFitError::kCannotGetMatrixInverse;
        return m_ErrorCode;
      }

      m_V_E = ((m_E.T()) * m_V_D * m_E).inverse(err_inverse);
      if (err_inverse) {
        m_ErrorCode = KFitError::kCannotGetMatrixInverse;
        return m_ErrorCode;
      }
      m_lam0 = m_V_D * (m_D * (m_al_0 - m_al_1) + m_d);
      chisq  = ((m_lam0.T()) * (m_D * (m_al_0 - m_al_1) + m_E * (m_v - m_v_a) + m_d))(1, 1);
      m_v_a  = m_v_a - m_V_E * (m_E.T()) * m_lam0;

      if (tmp_chisq <= chisq) {
        if (i == 0) {
          m_ErrorCode = KFitError::kBadInitialCHIsq;
          return m_ErrorCode;
        } else {
          chisq   = tmp_chisq;
          m_v_a   = tmp_v_a;
          m_V_E   = tmp_V_E;
          m_V_D   = tmp_V_D;
          m_lam0  = tmp_lam0;
          m_E     = tmp_E;
          m_D     = tmp_D;
          break;
        }
      } else {
        tmp_chisq = chisq;
        tmp_v_a   = m_v_a;
        tmp_V_E   = m_V_E;
        tmp_V_D   = m_V_D;
        tmp_lam0  = m_lam0;
        tmp_E     = m_E;
        tmp_D     = m_D;
        if (i == KFitConst::kMaxIterationCount - 1) {
          m_FlagOverIteration = true;
        }
      }
    } // i'th loop over


    m_al_a   = m_al_1;
    m_lam    = m_lam0 - m_V_D * m_E * m_V_E * (m_E.T()) * m_lam0;
    m_al_1   = m_al_0 - m_V_al_0 * (m_D.T()) * m_lam;

    if (j == 0) {

      tmp2_chisq = chisq;
      tmp2_v_a   = m_v_a;
      tmp2_v     = m_v;
      tmp2_V_E   = m_V_E;
      tmp2_V_D   = m_V_D;
      tmp2_lam0  = m_lam0;
      tmp2_E     = m_E;
      tmp2_D     = m_D;
      tmp_al_a   = m_al_a;

    } else {

      if (tmp2_chisq <= chisq) {
        chisq   = tmp2_chisq;
        m_v_a   = tmp2_v_a;
        m_v     = tmp2_v;
        m_V_E   = tmp2_V_E;
        m_V_D   = tmp2_V_D;
        m_lam0  = tmp2_lam0;
        m_E     = tmp2_E;
        m_D     = tmp2_D;
        m_al_a  = tmp_al_a;
        break;
      } else {
        tmp2_chisq = chisq;
        tmp2_v_a   = m_v_a;
        tmp2_v     = m_v;
        tmp2_V_E   = m_V_E;
        tmp2_V_D   = m_V_D;
        tmp2_lam0  = m_lam0;
        tmp2_E     = m_E;
        tmp2_D     = m_D;
        tmp_al_a   = m_al_a;
        if (j == KFitConst::kMaxIterationCount - 1) {
          m_FlagOverIteration = true;
        }
      }
    }
  } // j'th loop over


  if (m_ErrorCode != KFitError::kNoError) return m_ErrorCode;

  m_lam    = m_lam0 - m_V_D * m_E * m_V_E * (m_E.T()) * m_lam0;
  m_al_1   = m_al_0 - m_V_al_0 * (m_D.T()) * m_lam;
  m_V_Dt   = m_V_D  - m_V_D * m_E * m_V_E * (m_E.T()) * m_V_D;
  m_V_al_1 = m_V_al_0 - m_V_al_0 * (m_D.T()) * m_V_Dt * m_D * m_V_al_0;
  m_Cov_v_al_1 = -m_V_E * (m_E.T()) * m_V_D * m_D * m_V_al_0;

  if (prepareOutputMatrix() != KFitError::kNoError) return m_ErrorCode;

  m_CHIsq = chisq;

  m_FlagFitted = true;

  return m_ErrorCode = KFitError::kNoError;
}


bool
KFitBase::isFitted(void) const
{
  if (m_FlagFitted) return true;

  KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kNotFittedYet);

  return false;
}


bool
KFitBase::isTrackIDInRange(const int id) const
{
  if (0 <= id && id < m_TrackCount) return true;

  KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kOutOfRange);

  return false;
}


bool
KFitBase::isNonZeroEnergy(const HepLorentzVector& p) const
{
  if (p.t() != 0) return true;

  KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kDivisionByZero);

  return false;
}

