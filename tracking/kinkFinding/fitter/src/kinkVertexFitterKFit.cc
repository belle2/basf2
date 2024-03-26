/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: J. Tanaka                                        *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <algorithm>
#include <cstdio>

#include <TMatrixFSym.h>

//#include <analysis/VertexFitting/KFit/MakeMotherKFit.h>
#include <tracking/kinkFinding/fitter/kinkVertexFitterKFit.h>
#include <tracking/kinkFinding/utility/CLHEPToROOT.h>
#include <framework/gearbox/Const.h>

using namespace std;
using namespace Belle2;
using namespace CLHEP;

kinkVertexFitterKFit::kinkVertexFitterKFit():
  m_BeforeVertex(HepPoint3D(0, 0, 0)),
  m_AfterVertexError(HepSymMatrix(3, 0))
{
  m_CorrelationMode = false;
  m_FlagFitted = false;
  m_FlagKnownVertex = false;
  m_CHIsqVertex = 0;
  m_NecessaryTrackCount = 2;
  m_V_E = HepMatrix(3, 3, 0);
  m_v   = HepMatrix(3, 1, 0);
  m_v_a = HepMatrix(3, 1, 0);

  fill_n(m_EachCHIsq, KFitConst::kMaxTrackCount2, 0);
}

kinkVertexFitterKFit::~kinkVertexFitterKFit() = default;


enum KFitError::ECode
kinkVertexFitterKFit::setInitialVertex(const HepPoint3D& v) {
  m_BeforeVertex = v;

  return m_ErrorCode = KFitError::kNoError;
}

enum KFitError::ECode kinkVertexFitterKFit::setInitialVertex(const B2Vector3D& v)
{
  m_BeforeVertex = HepPoint3D(v.X(), v.Y(), v.Z());
  m_ErrorCode = KFitError::kNoError;
  return m_ErrorCode;
}


enum KFitError::ECode
kinkVertexFitterKFit::setKnownVertex(const bool flag) {
  m_FlagKnownVertex = flag;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
kinkVertexFitterKFit::setCorrelationMode(const bool m) {
  m_CorrelationMode = m;

  return m_ErrorCode = KFitError::kNoError;
}


const HepPoint3D
kinkVertexFitterKFit::getVertex(const int flag) const
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
kinkVertexFitterKFit::getVertexError() const
{
  return m_AfterVertexError;
}


double
kinkVertexFitterKFit::getCHIsq() const
{
  return KFitBase::getCHIsq();
}


double
kinkVertexFitterKFit::getCHIsqVertex() const
{
  // only m_FlagBeam = 1
  return m_CHIsqVertex;
}


const HepMatrix
kinkVertexFitterKFit::getTrackVertexError(const int id) const
{
  if (!isTrackIDInRange(id)) return HepMatrix(3, KFitConst::kNumber7, 0);
  return m_AfterTrackVertexError[id];
}


double
kinkVertexFitterKFit::getTrackCHIsq(const int id) const
{
  if (!isTrackIDInRange(id)) return -1;

  if (!m_FlagKnownVertex && m_CorrelationMode) {
    return KFitBase::getTrackCHIsq(id);
  }

  return m_EachCHIsq[id];
}


double
kinkVertexFitterKFit::getTrackPartCHIsq() const
{
  if (m_ErrorCode != KFitError::kNoError) {
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return -1;
  }

  if (m_TrackCount == 0) {
    KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kBadTrackSize);
    return -1;
  }

  double chisq = 0.0;
  for (int i = 0; i < m_TrackCount; i++) {
    const double i_chisq = this->getTrackCHIsq(i);
    chisq += i_chisq;
  }

  return chisq;
}


int
kinkVertexFitterKFit::getTrackPartNDF() const
{
  if (m_ErrorCode != KFitError::kNoError) {
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return 0;
  }

  if (m_TrackCount == 0) {
    KFitError::displayError(__FILE__, __LINE__, __func__, KFitError::kBadTrackSize);
    return 0;
  }

  return m_TrackCount * 2 - 2;
}


enum KFitError::ECode
kinkVertexFitterKFit::doFit() {

  if (m_FlagKnownVertex)
    m_ErrorCode = this->doFit5();
  else
    m_ErrorCode = this->doFit3();

  const enum KFitError::ECode tmp_ErrorCode = m_ErrorCode;

  if (tmp_ErrorCode == KFitError::kNoError) m_FlagFitted = true;

  return m_ErrorCode = tmp_ErrorCode;
}

enum KFitError::ECode
kinkVertexFitterKFit::doFit3() {
  // use small Matrix --> No Correlation
  if (m_ErrorCode != KFitError::kNoError) return m_ErrorCode;

  if (m_TrackCount < m_NecessaryTrackCount)
  {
    m_ErrorCode = KFitError::kBadTrackSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  if (prepareInputMatrix() != KFitError::kNoError) return m_ErrorCode;
  if (calculateNDF() != KFitError::kNoError) return m_ErrorCode;


  double chisq = 0;
  double tmp2_chisq = KFitConst::kInitialCHIsq;
  int err_inverse = 0;

  m_al_a = m_al_0;
  HepMatrix tmp_al_a(m_al_a);

  HepMatrix tmp_D(m_D), tmp_E(m_E);
  HepMatrix tmp_V_D(m_V_D), tmp_V_E(m_V_E);
  HepMatrix tmp_lam0(m_lam0), tmp_v_a(m_v_a);

  HepMatrix tmp2_D(m_D), tmp2_E(m_E);
  HepMatrix tmp2_V_D(m_V_D), tmp2_V_E(m_V_E);
  HepMatrix tmp2_lam0(m_lam0), tmp2_v_a(m_v_a);

  std::vector<double> tmp_each_chisq(m_TrackCount);
  std::vector<double> tmp2_each_chisq(m_TrackCount);

  for (int j = 0; j < KFitConst::kMaxIterationCount; j++)   // j'th loop start
  {

    double tmp_chisq = KFitConst::kInitialCHIsq;

    for (int i = 0; i < KFitConst::kMaxIterationCount; i++) { // i'th loop start

      if (prepareInputSubMatrix() != KFitError::kNoError) return m_ErrorCode;
      if (makeCoreMatrix() != KFitError::kNoError) return m_ErrorCode;

      HepMatrix tV_Ein(3, 3, 0);
      chisq = 0;

      for (int k = 0; k < m_TrackCount; k++) { // k'th loop start

        HepMatrix tD = m_D.sub(2 * k + 1, 2 * (k + 1), KFitConst::kNumber6 * k + 1,
                               KFitConst::kNumber6 * (k + 1)); // 2x6
        HepMatrix tV_D = ((m_V_al_0.sub(KFitConst::kNumber6 * k + 1,
                                        (int)(KFitConst::kNumber6 * (k + 1)))).similarity(tD)).inverse(
                                          err_inverse); // 2x2
        if (err_inverse) {
          m_ErrorCode = KFitError::kCannotGetMatrixInverse;
          KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
          return m_ErrorCode;
        }

        m_V_D.sub(2 * k + 1, 2 * k + 1, tV_D);
        HepMatrix tE = m_E.sub(2 * k + 1, 2 * (k + 1), 1, 3); // 2x3
        tV_Ein += (tE.T()) * tV_D * tE; // 3x3
        HepMatrix tDeltaAl = (m_al_0 - m_al_1).sub(KFitConst::kNumber6 * k + 1, KFitConst::kNumber6 * (k + 1),
                                                   1, 1); // 6x1
        HepMatrix td = m_d.sub(2 * k + 1, 2 * (k + 1), 1, 1); // 2x1
        HepMatrix tlam0 = tV_D * (tD * tDeltaAl + td); // 2x2x(2x6x6x1+2x1) = 2x1
        m_lam0.sub(2 * k + 1, 1, tlam0);
        m_EachCHIsq[k] = ((tlam0.T()) * (tD * tDeltaAl + tE * (m_v - m_v_a) + td))(1,
                         1); // 1x2x(2x6x6x1+2x3x3x1+2x1)
        chisq += m_EachCHIsq[k];
      } // k'th loop over

      m_V_E = tV_Ein.inverse(err_inverse);
      if (err_inverse) {
        m_ErrorCode = KFitError::kCannotGetMatrixInverse;
        KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
        return m_ErrorCode;
      }

      m_v_a = m_v_a - m_V_E * (m_E.T()) * m_lam0;

      if (tmp_chisq <= chisq) {
        if (i == 0) {
          m_ErrorCode = KFitError::kBadInitialCHIsq;
        } else {
          for (int k = 0; k < m_TrackCount; k++) m_EachCHIsq[k] = tmp_each_chisq[k];
          chisq = tmp_chisq;
          m_v_a = tmp_v_a;
          m_V_E = tmp_V_E;
          m_V_D = tmp_V_D;
          m_lam0 = tmp_lam0;
          m_E = tmp_E;
          m_D = tmp_D;
        }
        break;
      } else {
        for (int k = 0; k < m_TrackCount; k++) tmp_each_chisq[k] = m_EachCHIsq[k];
        tmp_chisq = chisq;
        tmp_v_a = m_v_a;
        tmp_V_E = m_V_E;
        tmp_V_D = m_V_D;
        tmp_lam0 = m_lam0;
        tmp_E = m_E;
        tmp_D = m_D;
        if (i == KFitConst::kMaxIterationCount - 1) {
          m_FlagOverIteration = true;
        }
      }
    } // i'th loop over

    m_al_a = m_al_1;
    m_lam = m_lam0 - m_V_D * m_E * m_V_E * (m_E.T()) * m_lam0;
    m_al_1 = m_al_0 - m_V_al_0 * (m_D.T()) * m_lam;

    if (j == 0) {

      for (int k = 0; k < m_TrackCount; k++) tmp2_each_chisq[k] = m_EachCHIsq[k];
      tmp2_chisq = chisq;
      tmp2_v_a = m_v_a;
      tmp2_V_E = m_V_E;
      tmp2_V_D = m_V_D;
      tmp2_lam0 = m_lam0;
      tmp2_E = m_E;
      tmp2_D = m_D;
      tmp_al_a = m_al_a;

    } else {

      if (tmp2_chisq <= chisq) {
        for (int k = 0; k < m_TrackCount; k++) m_EachCHIsq[k] = tmp2_each_chisq[k];
        chisq = tmp2_chisq;
        m_v_a = tmp2_v_a;
        m_V_E = tmp2_V_E;
        m_V_D = tmp2_V_D;
        m_lam0 = tmp2_lam0;
        m_E = tmp2_E;
        m_D = tmp2_D;
        m_al_a = tmp_al_a;
        break;
      } else {
        for (int k = 0; k < m_TrackCount; k++) tmp2_each_chisq[k] = m_EachCHIsq[k];
        tmp2_chisq = chisq;
        tmp2_v_a = m_v_a;
        tmp2_V_E = m_V_E;
        tmp2_V_D = m_V_D;
        tmp2_lam0 = m_lam0;
        tmp2_E = m_E;
        tmp2_D = m_D;
        tmp_al_a = m_al_a;
      }
    }
  } // j'th loop over


  if (m_ErrorCode != KFitError::kNoError) return m_ErrorCode;

  m_lam = m_lam0 - m_V_D * m_E * m_V_E * (m_E.T()) * m_lam0;
  m_al_1 = m_al_0 - m_V_al_0 * (m_D.T()) * m_lam;
  m_V_Dt = m_V_D - m_V_D * m_E * m_V_E * (m_E.T()) * m_V_D;
  m_V_al_1 = m_V_al_0 - m_V_al_0 * (m_D.T()) * m_V_Dt * m_D * m_V_al_0;
  m_Cov_v_al_1 = -m_V_E * (m_E.T()) * m_V_D * m_D * m_V_al_0;

  if (prepareOutputMatrix() != KFitError::kNoError) return m_ErrorCode;

  m_CHIsq = chisq;

  return m_ErrorCode = KFitError::kNoError;
}



enum KFitError::ECode
kinkVertexFitterKFit::doFit5() {
  // known vertex --> do not find vertex. (only no correlation)
  if (m_ErrorCode != KFitError::kNoError) return m_ErrorCode;

  if (m_TrackCount < m_NecessaryTrackCount - 1)
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

  m_al_a = m_al_0;
  HepMatrix tmp_al_a(m_al_a);

  HepMatrix tmp_al_0(m_al_1);
  HepMatrix tmp_V_al_0(m_V_al_1);

  std::vector<double> tmp_each_chisq(m_TrackCount);

  for (int i = 0; i < KFitConst::kMaxIterationCount; i++)
  {

    if (makeCoreMatrix() != KFitError::kNoError) return m_ErrorCode;

    chisq = 0;

    for (int k = 0; k < m_TrackCount; k++) {
      HepMatrix tD = m_D.sub(2 * k + 1, 2 * (k + 1), KFitConst::kNumber6 * k + 1, KFitConst::kNumber6 * (k + 1)); // 2x6
      HepMatrix tV_D = ((m_V_al_0.sub(KFitConst::kNumber6 * k + 1,
                                      (int)(KFitConst::kNumber6 * (k + 1)))).similarity(tD)).inverse(err_inverse); // 2x2
      if (err_inverse) {
        m_ErrorCode = KFitError::kCannotGetMatrixInverse;
        KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
        return m_ErrorCode;
      }
      m_V_D.sub(2 * k + 1, 2 * k + 1, tV_D);

      HepMatrix tDeltaAl = (m_al_0 - m_al_1).sub(KFitConst::kNumber6 * k + 1, KFitConst::kNumber6 * (k + 1), 1, 1); // 6x1
      HepMatrix td = m_d.sub(2 * k + 1, 2 * (k + 1), 1, 1); // 2x1
      HepMatrix tlam = tV_D * (tD * tDeltaAl + td); // 2x2x(2x6x6x1+2x1) = 2x1
      m_lam.sub(2 * k + 1, 1, tlam);
      m_EachCHIsq[k] = ((tlam.T()) * (tD * tDeltaAl + td))(1, 1); // 1x2x(2x6x6x1+2x1)
      chisq += m_EachCHIsq[k];
    }

    m_al_a   = m_al_1;
    m_al_1   = m_al_0 - m_V_al_0 * (m_D.T()) * m_lam;
    m_V_al_1 = m_V_al_0 - m_V_al_0 * (m_D.T()) * m_V_D * m_D * m_V_al_0;

    if (tmp_chisq <= chisq) {
      if (i == 0) {
        m_ErrorCode = KFitError::kBadInitialCHIsq;
      } else {
        for (int k = 0; k < m_TrackCount; k++) m_EachCHIsq[k] = tmp_each_chisq[k];
        chisq    = tmp_chisq;
        m_al_1   = tmp_al_0;
        m_V_al_1 = tmp_V_al_0;
        m_al_a   = tmp_al_a;
      }
      break;
    } else {
      for (int k = 0; k < m_TrackCount; k++) tmp_each_chisq[k] = m_EachCHIsq[k];
      tmp_chisq  = chisq;
      tmp_al_0   = m_al_1;
      tmp_V_al_0 = m_V_al_1;
      tmp_al_a   = m_al_a;
      if (i == KFitConst::kMaxIterationCount - 1) {
        m_FlagOverIteration = true;
      }
    }
  }


  if (m_ErrorCode != KFitError::kNoError) return m_ErrorCode;

  if (prepareOutputMatrix() != KFitError::kNoError) return m_ErrorCode;

  m_CHIsq = chisq;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
kinkVertexFitterKFit::prepareInputMatrix() {
  if (!m_CorrelationMode || m_FlagKnownVertex)
  {
    if (m_TrackCount > KFitConst::kMaxTrackCount2) {
      m_ErrorCode = KFitError::kBadTrackSize;
      KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
      return m_ErrorCode;
    }
  } else
  {
    if (m_TrackCount > KFitConst::kMaxTrackCount) {
      m_ErrorCode = KFitError::kBadTrackSize;
      KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
      return m_ErrorCode;
    }
  }


  int index = 0;
  HepMatrix    tmp_al_0(KFitConst::kNumber6 * m_TrackCount, 1, 0);
  HepSymMatrix tmp_V_al_0(KFitConst::kNumber6 * m_TrackCount, 0);
  HepMatrix    tmp_property(m_TrackCount, 3, 0);


  for (auto& track : m_Tracks)
  {
    // momentum x,y,z and position x,y,z
    for (int j = 0; j < KFitConst::kNumber6; j++)
      tmp_al_0[index * KFitConst::kNumber6 + j][0] = track.getFitParameter(j, KFitConst::kBeforeFit);
    // these error
    tmp_V_al_0.sub(index * KFitConst::kNumber6 + 1, track.getFitError(KFitConst::kBeforeFit));
    // charge , mass , a
    tmp_property[index][0] =  track.getCharge();
    tmp_property[index][1] =  track.getMass();
    const double c = Belle2::Const::speedOfLight * 1e-4;
    tmp_property[index][2] = -c * m_MagneticField * track.getCharge();
    index++;
  }

  // error between tarck and track
  m_V_al_0 = tmp_V_al_0;
  if (!m_FlagKnownVertex && m_CorrelationMode)
  {
    if (m_FlagCorrelation) {
      this->prepareCorrelation();
      if (m_ErrorCode != KFitError::kNoError) {
        KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
        return m_ErrorCode;
      }
    }
  }

  // vertex
  m_v_a[0][0] = m_BeforeVertex.x();
  m_v_a[1][0] = m_BeforeVertex.y();
  m_v_a[2][0] = m_BeforeVertex.z();

  // set member matrix
  m_al_0     = tmp_al_0;
  m_al_1     = m_al_0;
  m_property = tmp_property;

  // define size of matrix
  m_V_al_1 = HepMatrix(KFitConst::kNumber6 * m_TrackCount, KFitConst::kNumber6 * m_TrackCount, 0);
  m_D    = m_V_al_1.sub(1, m_TrackCount * 2, 1, KFitConst::kNumber6 * m_TrackCount);
  m_E    = m_V_al_1.sub(1, m_TrackCount * 2, 1, 3);
  m_d    = m_V_al_1.sub(1, m_TrackCount * 2, 1, 1);
  m_V_D  = m_V_al_1.sub(1, m_TrackCount * 2, 1, m_TrackCount * 2);
  m_lam  = m_V_al_1.sub(1, m_TrackCount * 2, 1, 1);
  m_lam0 = m_V_al_1.sub(1, m_TrackCount * 2, 1, 1);
  m_V_Dt = m_V_al_1.sub(1, m_TrackCount * 2, 1, m_TrackCount * 2);
  m_Cov_v_al_1 = m_V_al_1.sub(1, 3, 1, KFitConst::kNumber6 * m_TrackCount);

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
kinkVertexFitterKFit::prepareInputSubMatrix() {
  // vertex
  for (int i = 0; i < 3; i++)
  {
    m_v[i][0] = m_v_a[i][0];
  }
  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
kinkVertexFitterKFit::prepareOutputMatrix() {
  Hep3Vector h3v;
  unsigned index = 0;

  for (auto& pdata : m_Tracks)
  {
    // tracks
    // momentum
    h3v.setX(m_al_1[index * KFitConst::kNumber6 + 0][0]);
    h3v.setY(m_al_1[index * KFitConst::kNumber6 + 1][0]);
    h3v.setZ(m_al_1[index * KFitConst::kNumber6 + 2][0]);
    pdata.setMomentum(HepLorentzVector(h3v, sqrt(h3v.mag2() + pdata.getMass()*pdata.getMass())), KFitConst::kAfterFit);
    // position
    pdata.setPosition(HepPoint3D(
                        m_al_1[index * KFitConst::kNumber6 + 3][0],
                        m_al_1[index * KFitConst::kNumber6 + 4][0],
                        m_al_1[index * KFitConst::kNumber6 + 5][0]), KFitConst::kAfterFit);
    // error of the tracks
    pdata.setError(makeError1(pdata.getMomentum(),
                              m_V_al_1.sub(
                                index    * KFitConst::kNumber6 + 1,
                                (index + 1)*KFitConst::kNumber6,
                                index    * KFitConst::kNumber6 + 1,
                                (index + 1)*KFitConst::kNumber6)),
                   KFitConst::kAfterFit);
    if (m_ErrorCode != KFitError::kNoError) break;
    index++;
  }

  // vertex
  m_AfterVertex.setX(m_v_a[0][0]);
  m_AfterVertex.setY(m_v_a[1][0]);
  m_AfterVertex.setZ(m_v_a[2][0]);

  // error of the vertex
  for (int i = 0; i < 3; i++) for (int j = i; j < 3; j++)
    {
      m_AfterVertexError[i][j] = m_V_E[i][j];
    }

  // error between vertex and tracks
  for (int i = 0; i < m_TrackCount; i++)
  {
    HepMatrix hm(3, KFitConst::kNumber6, 0);
    for (int j = 0; j < 3; j++) for (int k = 0; k < KFitConst::kNumber6; k++) {
        hm[j][k] = m_Cov_v_al_1[j][KFitConst::kNumber6 * i + k];
      }
    m_AfterTrackVertexError.push_back(makeError2(m_Tracks[i].getMomentum(), hm));
  }

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
kinkVertexFitterKFit::makeCoreMatrix() {
  // vertex fit
  for (int i = 0; i < m_TrackCount; i++)
  {
    double S, U;
    double sininv;

    double px = m_al_1[i * KFitConst::kNumber6 + 0][0];
    double py = m_al_1[i * KFitConst::kNumber6 + 1][0];
    double pz = m_al_1[i * KFitConst::kNumber6 + 2][0];
    double x  = m_al_1[i * KFitConst::kNumber6 + 3][0];
    double y  = m_al_1[i * KFitConst::kNumber6 + 4][0];
    double z  = m_al_1[i * KFitConst::kNumber6 + 5][0];
    double a  = m_property[i][2];

    double pt =  sqrt(px * px + py * py);
    if (pt == 0) {
      m_ErrorCode = KFitError::kDivisionByZero;
      KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
      return m_ErrorCode;
    }

    double invPt   = 1. / pt;
    double invPt2  = invPt * invPt;
    double dlx  =  m_v_a[0][0] - x;
    double dly  =  m_v_a[1][0] - y;
    double dlz  =  m_v_a[2][0] - z;
    double a1   = -dlx * py + dly * px;
    double a2   =  dlx * px + dly * py;
    double r2d2 =  dlx * dlx + dly * dly;
    double Rx   =  dlx - 2.*px * a2 * invPt2;
    double Ry   =  dly - 2.*py * a2 * invPt2;

    if (a != 0) { // charged

      double B = a * a2 * invPt2;
      if (fabs(B) > 1) {
        m_ErrorCode = KFitError::kCannotGetARCSIN;
        B2DEBUG(10, "KFitError: Cannot calculate arcsin");
        //KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
        return m_ErrorCode;
      }
      // sin^(-1)(B)
      sininv = asin(B);
      double tmp0 = 1.0 - B * B;
      if (tmp0 == 0) {
        m_ErrorCode = KFitError::kDivisionByZero;
        KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
        return m_ErrorCode;
      }
      // 1/sqrt(1-B^2)
      double sqrtag = 1.0 / sqrt(tmp0);
      S = sqrtag * invPt2;
      U = dlz - pz * sininv / a;

    } else { // neutral

      sininv = 0.0;
      S = invPt2;
      U = dlz - pz * a2 * invPt2;
    }

    // d
    m_d[i * 2 + 0][0] = a1 - 0.5 * a * r2d2;
    m_d[i * 2 + 1][0] = U * pt;

    // D
    m_D[i * 2 + 0][i * KFitConst::kNumber6 + 0] =  dly;
    m_D[i * 2 + 0][i * KFitConst::kNumber6 + 1] = -dlx;
    m_D[i * 2 + 0][i * KFitConst::kNumber6 + 2] =  0.0;
    m_D[i * 2 + 0][i * KFitConst::kNumber6 + 3] =  py + a * dlx;
    m_D[i * 2 + 0][i * KFitConst::kNumber6 + 4] = -px + a * dly;
    m_D[i * 2 + 0][i * KFitConst::kNumber6 + 5] =  0.0;
    m_D[i * 2 + 1][i * KFitConst::kNumber6 + 0] = -pz * pt * S * Rx + U * px * invPt;
    m_D[i * 2 + 1][i * KFitConst::kNumber6 + 1] = -pz * pt * S * Ry + U * py * invPt;
    m_D[i * 2 + 1][i * KFitConst::kNumber6 + 2] = a != 0 ? -sininv * pt / a : -a2 * invPt;
    m_D[i * 2 + 1][i * KFitConst::kNumber6 + 3] =  px * pz * pt * S;
    m_D[i * 2 + 1][i * KFitConst::kNumber6 + 4] =  py * pz * pt * S;
    m_D[i * 2 + 1][i * KFitConst::kNumber6 + 5] = -pt;

    // E
    m_E[i * 2 + 0][0] = -py - a * dlx;
    m_E[i * 2 + 0][1] =  px - a * dly;
    m_E[i * 2 + 0][2] =  0.0;
    m_E[i * 2 + 1][0] = -px * pz * pt * S;
    m_E[i * 2 + 1][1] = -py * pz * pt * S;
    m_E[i * 2 + 1][2] =  pt;
  }

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
kinkVertexFitterKFit::calculateNDF() {
  if (m_FlagKnownVertex) m_NDF = 2 * m_TrackCount;
  else m_NDF = 2 * m_TrackCount - 3;

  return m_ErrorCode = KFitError::kNoError;
}

/*enum KFitError::ECode kinkVertexFitterKFit::updateMother(Particle* mother)
{
  MakeMotherKFit kmm;
  kmm.setMagneticField(m_MagneticField);
  unsigned n = getTrackCount();
  for (unsigned i = 0; i < n; ++i) {
    kmm.addTrack(getTrackMomentum(i), getTrackPosition(i), getTrackError(i),
                 getTrack(i).getCharge());
    kmm.setTrackVertexError(getTrackVertexError(i));
    for (unsigned j = i + 1; j < n; ++j) {
      kmm.setCorrelation(getCorrelation(i, j));
    }
  }
  kmm.setVertex(getVertex());
  kmm.setVertexError(getVertexError());
  m_ErrorCode = kmm.doMake();
  if (m_ErrorCode != KFitError::kNoError)
    return m_ErrorCode;
  double chi2 = getCHIsq();
  int ndf = getNDF();
  double prob = TMath::Prob(chi2, ndf);
  //
  bool haschi2 = mother->hasExtraInfo("chiSquared");
  if (haschi2) {
    mother->setExtraInfo("chiSquared", chi2);
    mother->setExtraInfo("ndf", ndf);
  } else {
    mother->addExtraInfo("chiSquared", chi2);
    mother->addExtraInfo("ndf", ndf);
  }

  mother->updateMomentum(
    CLHEPToROOT::getLorentzVector(kmm.getMotherMomentum()),
    CLHEPToROOT::getXYZVector(kmm.getMotherPosition()),
    CLHEPToROOT::getTMatrixFSym(kmm.getMotherError()),
    prob);
  m_ErrorCode = KFitError::kNoError;
  return m_ErrorCode;
}*/
