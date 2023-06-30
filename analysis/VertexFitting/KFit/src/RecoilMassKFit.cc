/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cstdio>

#include <analysis/VertexFitting/KFit/RecoilMassKFit.h>
#include <analysis/VertexFitting/KFit/MakeMotherKFit.h>
#include <analysis/utility/CLHEPToROOT.h>
#include <framework/gearbox/Const.h>

#include <TMath.h>
#include <TMatrixFSym.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::analysis;
using namespace CLHEP;
using namespace ROOT::Math;

RecoilMassKFit::RecoilMassKFit()
{
  m_FlagFitted = false;
  m_FlagTrackVertexError = false;
  m_FlagFitIncludingVertex = false;
  m_FlagAtDecayPoint = true;
  m_NecessaryTrackCount = 2;
  m_d   = HepMatrix(1, 1, 0);
  m_V_D = HepMatrix(1, 1, 0);
  m_lam = HepMatrix(1, 1, 0);
  m_AfterVertexError = HepSymMatrix(3, 0);
  m_recoilMass = -1.0;
  m_FourMomentum = PxPyPzEVector();
}


RecoilMassKFit::~RecoilMassKFit() = default;


enum KFitError::ECode
RecoilMassKFit::setVertex(const HepPoint3D& v) {
  m_BeforeVertex = v;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::setVertexError(const HepSymMatrix& e) {
  if (e.num_row() != 3)
  {
    m_ErrorCode = KFitError::kBadMatrixSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  m_BeforeVertexError = e;
  m_FlagFitIncludingVertex = true;

  return m_ErrorCode = KFitError::kNoError;
}

enum KFitError::ECode
RecoilMassKFit::setRecoilMass(const double m) {
  m_recoilMass = m;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::setFourMomentum(const PxPyPzEVector& m) {
  m_FourMomentum = m;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::setFlagAtDecayPoint(const bool flag) {
  m_FlagAtDecayPoint = flag;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::fixMass() {
  m_IsFixMass.push_back(true);

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::unfixMass() {
  m_IsFixMass.push_back(false);

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::setTrackVertexError(const HepMatrix& e) {
  if (e.num_row() != 3 || e.num_col() != KFitConst::kNumber7)
  {
    m_ErrorCode = KFitError::kBadMatrixSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  m_BeforeTrackVertexError.push_back(e);
  m_FlagTrackVertexError = true;
  m_FlagFitIncludingVertex = true;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::setTrackZeroVertexError() {
  HepMatrix zero(3, KFitConst::kNumber7, 0);

  return this->setTrackVertexError(zero);
}


enum KFitError::ECode
RecoilMassKFit::setCorrelation(const HepMatrix& m) {
  return KFitBase::setCorrelation(m);
}


enum KFitError::ECode
RecoilMassKFit::setZeroCorrelation() {
  return KFitBase::setZeroCorrelation();
}


const HepPoint3D
RecoilMassKFit::getVertex(const int flag) const
{
  if (flag == KFitConst::kAfterFit && !isFitted()) return HepPoint3D();

  switch (flag) {
    case KFitConst::kBeforeFit:
      return m_BeforeVertex;

    case KFitConst::kAfterFit:
      return m_AfterVertex;

    default:
      KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kOutOfRange);
      return HepPoint3D();
  }
}


const HepSymMatrix
RecoilMassKFit::getVertexError(const int flag) const
{
  if (flag == KFitConst::kAfterFit && !isFitted()) return HepSymMatrix(3, 0);

  if (flag == KFitConst::kBeforeFit)
    return m_BeforeVertexError;
  else if (flag == KFitConst::kAfterFit && m_FlagFitIncludingVertex)
    return m_AfterVertexError;
  else {
    KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kOutOfRange);
    return HepSymMatrix(3, 0);
  }
}


bool
RecoilMassKFit::getFlagAtDecayPoint() const
{
  return m_FlagAtDecayPoint;
}


bool
RecoilMassKFit::getFlagFitWithVertex() const
{
  return m_FlagFitIncludingVertex;
}


double
RecoilMassKFit::getCHIsq() const
{
  return m_CHIsq;
}


const HepMatrix
RecoilMassKFit::getTrackVertexError(const int id, const int flag) const
{
  if (flag == KFitConst::kAfterFit && !isFitted()) return HepMatrix(3, KFitConst::kNumber7, 0);
  if (!isTrackIDInRange(id)) return HepMatrix(3, KFitConst::kNumber7, 0);

  if (flag == KFitConst::kBeforeFit)
    return m_BeforeTrackVertexError[id];
  else if (flag == KFitConst::kAfterFit && m_FlagFitIncludingVertex)
    return m_AfterTrackVertexError[id];
  else {
    KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kOutOfRange);
    return HepMatrix(3, KFitConst::kNumber7, 0);
  }
}


double
RecoilMassKFit::getTrackCHIsq(const int id) const
{
  if (!isFitted()) return -1;
  if (!isTrackIDInRange(id)) return -1;

  if (m_IsFixMass[id]) {

    HepMatrix da(m_Tracks[id].getFitParameter(KFitConst::kBeforeFit) - m_Tracks[id].getFitParameter(KFitConst::kAfterFit));
    int err_inverse = 0;
    const double chisq = (da.T() * (m_Tracks[id].getFitError(KFitConst::kBeforeFit).inverse(err_inverse)) * da)[0][0];

    if (err_inverse) {
      KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kCannotGetMatrixInverse);
      return -1;
    }

    return chisq;

  } else {

    HepMatrix da(m_Tracks[id].getMomPos(KFitConst::kBeforeFit) - m_Tracks[id].getMomPos(KFitConst::kAfterFit));
    int err_inverse = 0;
    const double chisq = (da.T() * (m_Tracks[id].getError(KFitConst::kBeforeFit).inverse(err_inverse)) * da)[0][0];

    if (err_inverse) {
      KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kCannotGetMatrixInverse);
      return -1;
    }

    return chisq;

  }
}


const HepMatrix
RecoilMassKFit::getCorrelation(const int id1, const int id2, const int flag) const
{
  if (flag == KFitConst::kAfterFit && !isFitted()) return HepMatrix(KFitConst::kNumber7, KFitConst::kNumber7, 0);
  if (!isTrackIDInRange(id1)) return HepMatrix(KFitConst::kNumber7, KFitConst::kNumber7, 0);
  if (!isTrackIDInRange(id2)) return HepMatrix(KFitConst::kNumber7, KFitConst::kNumber7, 0);

  switch (flag) {
    case KFitConst::kBeforeFit:
      return KFitBase::getCorrelation(id1, id2, flag);

    case KFitConst::kAfterFit:
      return makeError3(
               this->getTrackMomentum(id1),
               this->getTrackMomentum(id2),
               m_V_al_1.sub(KFitConst::kNumber7 * id1 + 1, KFitConst::kNumber7 * (id1 + 1), KFitConst::kNumber7 * id2 + 1,
                            KFitConst::kNumber7 * (id2 + 1)),
               m_IsFixMass[id1],
               m_IsFixMass[id2]);

    default:
      KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kOutOfRange);
      return HepMatrix(KFitConst::kNumber7, KFitConst::kNumber7, 0);
  }
}


enum KFitError::ECode
RecoilMassKFit::doFit() {
  return KFitBase::doFit1();
}


enum KFitError::ECode
RecoilMassKFit::prepareInputMatrix() {
  if (m_TrackCount > KFitConst::kMaxTrackCount)
  {
    m_ErrorCode = KFitError::kBadTrackSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }


  if (m_IsFixMass.size() == 0)
  {
    // If no fix_mass flag at all,
    // all tracks are considered to be fixed at mass.
    for (int i = 0; i < m_TrackCount; i++) this->fixMass();
  } else if (m_IsFixMass.size() != (unsigned int)m_TrackCount)
  {
    m_ErrorCode = KFitError::kBadTrackSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }


  if (!m_FlagFitIncludingVertex)
  {
    int index = 0;
    m_al_0     = HepMatrix(KFitConst::kNumber7 * m_TrackCount, 1, 0);
    m_property = HepMatrix(m_TrackCount, 3, 0);
    m_V_al_0   = HepSymMatrix(KFitConst::kNumber7 * m_TrackCount, 0);

    for (auto& track : m_Tracks) {
      // momentum x,y,z and position x,y,z
      m_al_0[index * KFitConst::kNumber7 + 0][0] = track.getMomentum(KFitConst::kBeforeFit).x();
      m_al_0[index * KFitConst::kNumber7 + 1][0] = track.getMomentum(KFitConst::kBeforeFit).y();
      m_al_0[index * KFitConst::kNumber7 + 2][0] = track.getMomentum(KFitConst::kBeforeFit).z();
      m_al_0[index * KFitConst::kNumber7 + 3][0] = track.getMomentum(KFitConst::kBeforeFit).t();
      m_al_0[index * KFitConst::kNumber7 + 4][0] = track.getPosition(KFitConst::kBeforeFit).x();
      m_al_0[index * KFitConst::kNumber7 + 5][0] = track.getPosition(KFitConst::kBeforeFit).y();
      m_al_0[index * KFitConst::kNumber7 + 6][0] = track.getPosition(KFitConst::kBeforeFit).z();
      // these error
      m_V_al_0.sub(index * KFitConst::kNumber7 + 1, track.getError(KFitConst::kBeforeFit));
      // charge, mass, a
      m_property[index][0] =  track.getCharge();
      m_property[index][1] =  track.getMass();
      const double c = Belle2::Const::speedOfLight * 1e-4;
      m_property[index][2] = -c * m_MagneticField * track.getCharge();
      index++;
    }

    // error between track and track
    if (m_FlagCorrelation) {
      this->prepareCorrelation();
      if (m_ErrorCode != KFitError::kNoError) {
        KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
        return m_ErrorCode;
      }
    }

    // set member matrix
    m_al_1   = m_al_0;

    // define size of matrix
    m_V_al_1 = HepMatrix(KFitConst::kNumber7 * m_TrackCount, KFitConst::kNumber7 * m_TrackCount, 0);
    m_D      = m_V_al_1.sub(1, 1, 1, KFitConst::kNumber7 * m_TrackCount);

  } else
  {
    //TODO: Not Implemented
    return m_ErrorCode = KFitError::kUnimplemented;
  }

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::prepareInputSubMatrix() { // unused
  char buf[1024];
  sprintf(buf, "%s:%s(): internal error; this function should never be called", __FILE__, __func__);
  B2FATAL(buf);

  /* NEVER REACHEd HERE */
  return KFitError::kOutOfRange;
}


enum KFitError::ECode
RecoilMassKFit::prepareCorrelation() {
  if (m_BeforeCorrelation.size() != static_cast<unsigned int>(m_TrackCount * (m_TrackCount - 1) / 2))
  {
    m_ErrorCode = KFitError::kBadCorrelationSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  int row = 0, col = 0;

  for (auto& hm : m_BeforeCorrelation)
  {
    // counter
    row++;
    if (row == m_TrackCount) {
      col++;
      row = col + 1;
    }

    int ii = 0, jj = 0;
    for (int i = KFitConst::kNumber7 * row; i < KFitConst::kNumber7 * (row + 1); i++) {
      for (int j = KFitConst::kNumber7 * col; j < KFitConst::kNumber7 * (col + 1); j++) {
        m_V_al_0[i][j] = hm[ii][jj];
        jj++;
      }
      jj = 0;
      ii++;
    }
  }

  if (m_FlagFitIncludingVertex)
  {
    //TODO: Not Implemented
    return m_ErrorCode = KFitError::kUnimplemented;

    // ...error of vertex
    m_V_al_0.sub(KFitConst::kNumber7 * m_TrackCount + 1, m_BeforeVertexError);

    // ...error matrix between vertex and tracks
    if (m_FlagTrackVertexError) {
      if (m_BeforeTrackVertexError.size() != (unsigned int)m_TrackCount) {
        m_ErrorCode = KFitError::kBadCorrelationSize;
        KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
        return m_ErrorCode;
      }

      int i = 0;
      for (auto& hm : m_BeforeTrackVertexError) {
        for (int j = 0; j < 3; j++) for (int k = 0; k < KFitConst::kNumber7; k++) {
            m_V_al_0[j + KFitConst::kNumber7 * m_TrackCount][k + i * KFitConst::kNumber7] = hm[j][k];
          }
        i++;
      }
    }
  }

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::prepareOutputMatrix() {
  Hep3Vector h3v;
  int index = 0;
  for (auto& pdata : m_Tracks)
  {
    // tracks
    // momentum
    h3v.setX(m_al_1[index * KFitConst::kNumber7 + 0][0]);
    h3v.setY(m_al_1[index * KFitConst::kNumber7 + 1][0]);
    h3v.setZ(m_al_1[index * KFitConst::kNumber7 + 2][0]);
    if (m_IsFixMass[index])
      pdata.setMomentum(HepLorentzVector(h3v, sqrt(h3v.mag2() + pdata.getMass()*pdata.getMass())), KFitConst::kAfterFit);
    else
      pdata.setMomentum(HepLorentzVector(h3v, m_al_1[index * KFitConst::kNumber7 + 3][0]), KFitConst::kAfterFit);
    // position
    pdata.setPosition(HepPoint3D(
                        m_al_1[index * KFitConst::kNumber7 + 4][0],
                        m_al_1[index * KFitConst::kNumber7 + 5][0],
                        m_al_1[index * KFitConst::kNumber7 + 6][0]), KFitConst::kAfterFit);
    // error of the tracks
    pdata.setError(this->makeError3(pdata.getMomentum(),
                                    m_V_al_1.sub(
                                      index    * KFitConst::kNumber7 + 1,
                                      (index + 1)*KFitConst::kNumber7,
                                      index    * KFitConst::kNumber7 + 1,
                                      (index + 1)*KFitConst::kNumber7), m_IsFixMass[index]),
                   KFitConst::kAfterFit);
    if (m_ErrorCode != KFitError::kNoError) break;
    index++;
  }

  if (m_FlagFitIncludingVertex)
  {
    //TODO: Not Implemented
    return m_ErrorCode = KFitError::kUnimplemented;
  } else
  {
    // not fit
    m_AfterVertex = m_BeforeVertex;
  }

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::makeCoreMatrix() {
  if (!m_FlagFitIncludingVertex)
  {

    HepMatrix al_1_prime(m_al_1);
    HepMatrix Sum_al_1(4, 1, 0);
    std::vector<double> energy(m_TrackCount);
    double a;

    for (int i = 0; i < m_TrackCount; i++) {
      a = m_property[i][2];
      if (!m_FlagAtDecayPoint) a = 0.;
      al_1_prime[i * KFitConst::kNumber7 + 0][0] -= a * (m_BeforeVertex.y() - al_1_prime[i * KFitConst::kNumber7 + 5][0]);
      al_1_prime[i * KFitConst::kNumber7 + 1][0] += a * (m_BeforeVertex.x() - al_1_prime[i * KFitConst::kNumber7 + 4][0]);
      energy[i] = sqrt(al_1_prime[i * KFitConst::kNumber7 + 0][0] * al_1_prime[i * KFitConst::kNumber7 + 0][0] +
                       al_1_prime[i * KFitConst::kNumber7 + 1][0] * al_1_prime[i * KFitConst::kNumber7 + 1][0] +
                       al_1_prime[i * KFitConst::kNumber7 + 2][0] * al_1_prime[i * KFitConst::kNumber7 + 2][0] +
                       m_property[i][1] * m_property[i][1]);
      if (m_IsFixMass[i])
        Sum_al_1[3][0] += energy[i];
      else
        Sum_al_1[3][0] += al_1_prime[i * KFitConst::kNumber7 + 3][0];
    }

    for (int i = 0; i < m_TrackCount; i++) {
      for (int j = 0; j < 3; j++) Sum_al_1[j][0] += al_1_prime[i * KFitConst::kNumber7 + j][0];
    }

    Sum_al_1[0][0] -= m_FourMomentum.Px();
    Sum_al_1[1][0] -= m_FourMomentum.Py();
    Sum_al_1[2][0] -= m_FourMomentum.Pz();
    Sum_al_1[3][0] -= m_FourMomentum.E();

    m_d[0][0] =
      + Sum_al_1[3][0] * Sum_al_1[3][0] - Sum_al_1[0][0] * Sum_al_1[0][0]
      - Sum_al_1[1][0] * Sum_al_1[1][0] - Sum_al_1[2][0] * Sum_al_1[2][0]
      - m_recoilMass * m_recoilMass;

    for (int i = 0; i < m_TrackCount; i++) {
      if (energy[i] == 0) {
        m_ErrorCode = KFitError::kDivisionByZero;
        KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
        break;
      }

      a = m_property[i][2];
      if (!m_FlagAtDecayPoint) a = 0.;

      if (m_IsFixMass[i]) {
        double invE = 1. / energy[i];
        m_D[0][i * KFitConst::kNumber7 + 0] = 2.*(Sum_al_1[3][0] * al_1_prime[i * KFitConst::kNumber7 + 0][0] * invE - Sum_al_1[0][0]);
        m_D[0][i * KFitConst::kNumber7 + 1] = 2.*(Sum_al_1[3][0] * al_1_prime[i * KFitConst::kNumber7 + 1][0] * invE - Sum_al_1[1][0]);
        m_D[0][i * KFitConst::kNumber7 + 2] = 2.*(Sum_al_1[3][0] * al_1_prime[i * KFitConst::kNumber7 + 2][0] * invE - Sum_al_1[2][0]);
        m_D[0][i * KFitConst::kNumber7 + 3] = 0.;
        m_D[0][i * KFitConst::kNumber7 + 4] = -2.*(Sum_al_1[3][0] * al_1_prime[i * KFitConst::kNumber7 + 1][0] * invE - Sum_al_1[1][0]) * a;
        m_D[0][i * KFitConst::kNumber7 + 5] = 2.*(Sum_al_1[3][0] * al_1_prime[i * KFitConst::kNumber7 + 0][0] * invE - Sum_al_1[0][0]) * a;
        m_D[0][i * KFitConst::kNumber7 + 6] = 0.;
      } else {
        m_D[0][i * KFitConst::kNumber7 + 0] = -2.*Sum_al_1[0][0];
        m_D[0][i * KFitConst::kNumber7 + 1] = -2.*Sum_al_1[1][0];
        m_D[0][i * KFitConst::kNumber7 + 2] = -2.*Sum_al_1[2][0];
        m_D[0][i * KFitConst::kNumber7 + 3] =  2.*Sum_al_1[3][0];
        m_D[0][i * KFitConst::kNumber7 + 4] =  2.*Sum_al_1[1][0] * a;
        m_D[0][i * KFitConst::kNumber7 + 5] = -2.*Sum_al_1[0][0] * a;
        m_D[0][i * KFitConst::kNumber7 + 6] =  0.;
      }
    }

  } else
  {
    //TODO: Not Implemented
    return m_ErrorCode = KFitError::kUnimplemented;
  }

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
RecoilMassKFit::calculateNDF() {
  m_NDF = 1;

  return m_ErrorCode = KFitError::kNoError;
}

enum KFitError::ECode RecoilMassKFit::updateMother(Particle* mother)
{
  MakeMotherKFit kmm;
  kmm.setMagneticField(m_MagneticField);
  unsigned n = getTrackCount();
  for (unsigned i = 0; i < n; ++i) {
    kmm.addTrack(getTrackMomentum(i), getTrackPosition(i), getTrackError(i),
                 getTrack(i).getCharge());
    if (getFlagFitWithVertex())
      kmm.setTrackVertexError(getTrackVertexError(i));
    for (unsigned j = i + 1; j < n; ++j) {
      kmm.setCorrelation(getCorrelation(i, j));
    }
  }
  kmm.setVertex(getVertex());
  if (getFlagFitWithVertex())
    kmm.setVertexError(getVertexError());
  m_ErrorCode = kmm.doMake();
  if (m_ErrorCode != KFitError::kNoError)
    return m_ErrorCode;
  double chi2 = getCHIsq();
  int ndf = getNDF();
  double prob = TMath::Prob(chi2, ndf);
  //
  mother->writeExtraInfo("chiSquared", chi2);
  mother->writeExtraInfo("ndf", ndf);

  mother->updateMomentum(
    CLHEPToROOT::getLorentzVector(kmm.getMotherMomentum()),
    CLHEPToROOT::getXYZVector(kmm.getMotherPosition()),
    CLHEPToROOT::getTMatrixFSym(kmm.getMotherError()),
    prob);
  m_ErrorCode = KFitError::kNoError;
  return m_ErrorCode;
}
