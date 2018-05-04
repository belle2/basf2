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

#include <analysis/KFit/MakeMotherKFit.h>
#include <analysis/KFit/MassVertexFitKFit.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::analysis;
using namespace CLHEP;

MassVertexFitKFit::MassVertexFitKFit(void):
  m_BeforeVertex(HepPoint3D(0., 0., 0.)),
  m_AfterVertexError(HepSymMatrix(3, 0))
{
  m_FlagFitted = false;
  m_NecessaryTrackCount = 2;
  m_V_E = HepMatrix(3, 3, 0);
  m_v   = HepMatrix(3, 1, 0);
  m_v_a = HepMatrix(3, 1, 0);
  m_InvariantMass = -1.0;
}


MassVertexFitKFit::~MassVertexFitKFit(void)
{
}


enum KFitError::ECode
MassVertexFitKFit::setInitialVertex(const HepPoint3D& v) {
  m_BeforeVertex = v;

  return m_ErrorCode = KFitError::kNoError;
}

enum KFitError::ECode MassVertexFitKFit::setInitialVertex(const TVector3& v)
{
  m_BeforeVertex = HepPoint3D(v.X(), v.Y(), v.Z());
  m_ErrorCode = KFitError::kNoError;
  return m_ErrorCode;
}

enum KFitError::ECode
MassVertexFitKFit::setInvariantMass(const double m) {
  m_InvariantMass = m;

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MassVertexFitKFit::fixMass(void) {
  m_IsFixMass.push_back(true);

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MassVertexFitKFit::unfixMass(void) {
  m_IsFixMass.push_back(false);

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MassVertexFitKFit::setCorrelation(const HepMatrix& m) {
  return KFitBase::setCorrelation(m);
}


enum KFitError::ECode
MassVertexFitKFit::setZeroCorrelation(void) {
  return KFitBase::setZeroCorrelation();
}


const HepPoint3D
MassVertexFitKFit::getVertex(const int flag) const
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
MassVertexFitKFit::getVertexError(void) const
{
  return m_AfterVertexError;
}


double
MassVertexFitKFit::getInvariantMass(void) const
{
  return m_InvariantMass;
}


double
MassVertexFitKFit::getCHIsq(void) const
{
  return m_CHIsq;
}


const HepMatrix
MassVertexFitKFit::getTrackVertexError(const int id) const
{
  if (!isTrackIDInRange(id)) return HepMatrix(3, KFitConst::kNumber7, 0);

  return m_AfterTrackVertexError[id];
}


double
MassVertexFitKFit::getTrackCHIsq(const int id) const
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
MassVertexFitKFit::getCorrelation(const int id1, const int id2, const int flag) const
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
MassVertexFitKFit::doFit(void) {
  return KFitBase::doFit2();
}


enum KFitError::ECode
MassVertexFitKFit::prepareInputMatrix(void) {
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


  int index = 0;
  m_al_0     = HepMatrix(KFitConst::kNumber7 * m_TrackCount, 1, 0);
  m_property = HepMatrix(m_TrackCount, 3, 0);
  m_V_al_0   = HepSymMatrix(KFitConst::kNumber7 * m_TrackCount, 0);

  for (vector<KFitTrack>::const_iterator it = m_Tracks.begin(), endIt = m_Tracks.end(); it != endIt; ++it)
  {
    // momentum x,y,z and position x,y,z
    m_al_0[index * KFitConst::kNumber7 + 0][0] = it->getMomentum(KFitConst::kBeforeFit).x();
    m_al_0[index * KFitConst::kNumber7 + 1][0] = it->getMomentum(KFitConst::kBeforeFit).y();
    m_al_0[index * KFitConst::kNumber7 + 2][0] = it->getMomentum(KFitConst::kBeforeFit).z();
    m_al_0[index * KFitConst::kNumber7 + 3][0] = it->getMomentum(KFitConst::kBeforeFit).t();
    m_al_0[index * KFitConst::kNumber7 + 4][0] = it->getPosition(KFitConst::kBeforeFit).x();
    m_al_0[index * KFitConst::kNumber7 + 5][0] = it->getPosition(KFitConst::kBeforeFit).y();
    m_al_0[index * KFitConst::kNumber7 + 6][0] = it->getPosition(KFitConst::kBeforeFit).z();
    // these error
    m_V_al_0.sub(index * KFitConst::kNumber7 + 1, it->getError(KFitConst::kBeforeFit));
    // charge, mass, a
    m_property[index][0] =  it->getCharge();
    m_property[index][1] =  it->getMass();
    const double c = KFitConst::kLightSpeed; // C++ bug?
    // m_property[index][2] = -KFitConst::kLightSpeed * m_MagneticField * it->getCharge();
    m_property[index][2] = -c * m_MagneticField * it->getCharge();
    index++;
  }

  // error between track and track
  if (m_FlagCorrelation)
  {
    this->prepareCorrelation();
    if (m_ErrorCode != KFitError::kNoError) {
      KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
      return m_ErrorCode;
    }
  }

  // vertex
  m_v_a[0][0] = m_BeforeVertex.x();
  m_v_a[1][0] = m_BeforeVertex.y();
  m_v_a[2][0] = m_BeforeVertex.z();

  // set member matrix
  m_al_1     = m_al_0;

  m_V_al_1     = HepMatrix(KFitConst::kNumber7 * m_TrackCount, KFitConst::kNumber7 * m_TrackCount, 0);
  m_D          = m_V_al_1.sub(1, m_TrackCount * 2 + 1, 1, KFitConst::kNumber7 * m_TrackCount);
  m_E          = m_V_al_1.sub(1, m_TrackCount * 2 + 1, 1, 3);
  m_d          = m_V_al_1.sub(1, m_TrackCount * 2 + 1, 1, 1);
  m_V_D        = m_V_al_1.sub(1, m_TrackCount * 2 + 1, 1, m_TrackCount * 2 + 1);
  m_lam        = m_V_al_1.sub(1, m_TrackCount * 2 + 1, 1, 1);
  m_lam0       = m_V_al_1.sub(1, m_TrackCount * 2 + 1, 1, 1);
  m_V_Dt       = m_V_al_1.sub(1, m_TrackCount * 2 + 1, 1, m_TrackCount * 2 + 1);
  m_Cov_v_al_1 = m_V_al_1.sub(1, 3, 1, KFitConst::kNumber7 * m_TrackCount);

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MassVertexFitKFit::prepareInputSubMatrix(void) {
  // vertex
  for (int i = 0; i < 3; i++) m_v[i][0] = m_v_a[i][0];

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MassVertexFitKFit::prepareCorrelation(void) {
  if (m_BeforeCorrelation.size() != static_cast<unsigned int>(m_TrackCount * (m_TrackCount - 1) / 2))
  {
    m_ErrorCode = KFitError::kBadCorrelationSize;
    KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
    return m_ErrorCode;
  }

  int row = 0, col = 0;

  for (vector<HepMatrix>::const_iterator it = m_BeforeCorrelation.begin(), endIt = m_BeforeCorrelation.end(); it != endIt; ++it)
  {
    const HepMatrix& hm = *it;

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

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MassVertexFitKFit::prepareOutputMatrix(void) {
  Hep3Vector h3v;
  int index = 0;
  for (vector<KFitTrack>::iterator it = m_Tracks.begin(), endIt = m_Tracks.end(); it != endIt; ++it)
  {
    KFitTrack& pdata = *it;
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
    HepMatrix hm(3, KFitConst::kNumber7, 0);
    for (int j = 0; j < 3; j++)  for (int k = 0; k < KFitConst::kNumber7; k++) {
        hm[j][k] = m_Cov_v_al_1[j][KFitConst::kNumber7 * i + k];
      }
    if (m_IsFixMass[i])
      m_AfterTrackVertexError.push_back(this->makeError4(m_Tracks[i].getMomentum(), hm));
    else
      m_AfterTrackVertexError.push_back(hm);
  }

  return m_ErrorCode = KFitError::kNoError;
}


enum KFitError::ECode
MassVertexFitKFit::makeCoreMatrix(void) {
  // Mass Constraint
  HepMatrix al_1_prime(m_al_1);
  HepMatrix Sum_al_1(4, 1, 0);
  double energy[KFitConst::kMaxTrackCount2];
  double a;

  for (int i = 0; i < m_TrackCount; i++)
  {
    a = m_property[i][2];
    al_1_prime[i * KFitConst::kNumber7 + 0][0] -= a * (m_v_a[1][0] - al_1_prime[i * KFitConst::kNumber7 + 5][0]);
    al_1_prime[i * KFitConst::kNumber7 + 1][0] += a * (m_v_a[0][0] - al_1_prime[i * KFitConst::kNumber7 + 4][0]);
    energy[i] = sqrt(al_1_prime[i * KFitConst::kNumber7 + 0][0] * al_1_prime[i * KFitConst::kNumber7 + 0][0] +
    al_1_prime[i * KFitConst::kNumber7 + 1][0] * al_1_prime[i * KFitConst::kNumber7 + 1][0] +
    al_1_prime[i * KFitConst::kNumber7 + 2][0] * al_1_prime[i * KFitConst::kNumber7 + 2][0] +
    m_property[i][1] * m_property[i][1]);
  }

  for (int i = 0; i < m_TrackCount; i++)
  {
    if (m_IsFixMass[i])
      Sum_al_1[3][0] += energy[i];
    else
      Sum_al_1[3][0] += al_1_prime[i * KFitConst::kNumber7 + 3][0];

    for (int j = 0; j < 3; j++) Sum_al_1[j][0] += al_1_prime[i * KFitConst::kNumber7 + j][0];
  }

  m_d[2 * m_TrackCount][0] =
  + Sum_al_1[3][0] * Sum_al_1[3][0] - Sum_al_1[0][0] * Sum_al_1[0][0]
  - Sum_al_1[1][0] * Sum_al_1[1][0] - Sum_al_1[2][0] * Sum_al_1[2][0]
  - m_InvariantMass * m_InvariantMass;

  double Sum_a = 0., Sum_tmpx = 0., Sum_tmpy = 0.;
  for (int i = 0; i < m_TrackCount; i++)
  {
    if (energy[i] == 0) {
      m_ErrorCode = KFitError::kDivisionByZero;
      KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
      return m_ErrorCode;
    }

    a = m_property[i][2];

    if (m_IsFixMass[i]) {
      double invE = 1. / energy[i];
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 0] = 2.*(Sum_al_1[3][0] * al_1_prime[i * KFitConst::kNumber7 + 0][0] * invE -
                                                               Sum_al_1[0][0]);
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 1] = 2.*(Sum_al_1[3][0] * al_1_prime[i * KFitConst::kNumber7 + 1][0] * invE -
                                                               Sum_al_1[1][0]);
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 2] = 2.*(Sum_al_1[3][0] * al_1_prime[i * KFitConst::kNumber7 + 2][0] * invE -
                                                               Sum_al_1[2][0]);
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 3] = 0.;
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 4] = -2.*(Sum_al_1[3][0] * al_1_prime[i * KFitConst::kNumber7 + 1][0] * invE -
                                                                Sum_al_1[1][0]) * a;
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 5] = 2.*(Sum_al_1[3][0] * al_1_prime[i * KFitConst::kNumber7 + 0][0] * invE -
                                                               Sum_al_1[0][0]) * a;
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 6] = 0.;
      Sum_tmpx += al_1_prime[i * KFitConst::kNumber7 + 0][0] * invE * a;
      Sum_tmpy += al_1_prime[i * KFitConst::kNumber7 + 1][0] * invE * a;
    } else {
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 0] = -2.*Sum_al_1[0][0];
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 1] = -2.*Sum_al_1[1][0];
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 2] = -2.*Sum_al_1[2][0];
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 3] =  2.*Sum_al_1[3][0];
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 4] =  2.*Sum_al_1[1][0] * a;
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 5] = -2.*Sum_al_1[0][0] * a;
      m_D[2 * m_TrackCount][i * KFitConst::kNumber7 + 6] =  0.;
    }
    Sum_a    += a;
  }

  // m_E
  m_E[2 * m_TrackCount][0] = -2.*Sum_al_1[1][0] * Sum_a + 2.*Sum_al_1[3][0] * Sum_tmpy;
  m_E[2 * m_TrackCount][1] =  2.*Sum_al_1[0][0] * Sum_a - 2.*Sum_al_1[3][0] * Sum_tmpx;
  m_E[2 * m_TrackCount][2] =  0.;

  for (int i = 0; i < m_TrackCount; i++)
  {
    double B, S, U;
    double sininv, sqrtag;

    double px = m_al_1[i * KFitConst::kNumber7 + 0][0];
    double py = m_al_1[i * KFitConst::kNumber7 + 1][0];
    double pz = m_al_1[i * KFitConst::kNumber7 + 2][0];
    double x  = m_al_1[i * KFitConst::kNumber7 + 4][0];
    double y  = m_al_1[i * KFitConst::kNumber7 + 5][0];
    double z  = m_al_1[i * KFitConst::kNumber7 + 6][0];
    a  = m_property[i][2];

    double pt =  sqrt(px * px + py * py);

    if (pt == 0) {
      m_ErrorCode = KFitError::kDivisionByZero;
      KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
      return m_ErrorCode;
    }

    double invPt  =  1. / pt;
    double invPt2 =  invPt * invPt;
    double dlx    =  m_v_a[0][0] - x;
    double dly    =  m_v_a[1][0] - y;
    double dlz    =  m_v_a[2][0] - z;
    double a1     = -dlx * py + dly * px;
    double a2     =  dlx * px + dly * py;
    double r2d2   =  dlx * dlx + dly * dly;
    double Rx     =  dlx - 2.*px * a2 * invPt2;
    double Ry     =  dly - 2.*py * a2 * invPt2;

    if (a != 0.) { // charged

      B = a * a2 * invPt2;
      if (fabs(B) > 1.) {
        m_ErrorCode = KFitError::kDivisionByZero;
        KFitError::displayError(__FILE__, __LINE__, __func__, m_ErrorCode);
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
      sqrtag = 1.0 / sqrt(tmp0);
      S = sqrtag * invPt2;
      U = dlz - pz * sininv / a;

    } else { // neutral

      B      = 0.0;
      sininv = 0.0;
      sqrtag = 1.0;
      S = invPt2;
      U = dlz - pz * a2 * invPt2;

    }

    // d
    m_d[i * 2 + 0][0] = a1 - 0.5 * a * r2d2;
    m_d[i * 2 + 1][0] = U * pt;

    // D
    m_D[i * 2 + 0][i * KFitConst::kNumber7 + 0] =  dly;
    m_D[i * 2 + 0][i * KFitConst::kNumber7 + 1] = -dlx;
    m_D[i * 2 + 0][i * KFitConst::kNumber7 + 2] =  0.0;
    m_D[i * 2 + 0][i * KFitConst::kNumber7 + 4] =  py + a * dlx;
    m_D[i * 2 + 0][i * KFitConst::kNumber7 + 5] = -px + a * dly;
    m_D[i * 2 + 0][i * KFitConst::kNumber7 + 6] =  0.0;
    m_D[i * 2 + 1][i * KFitConst::kNumber7 + 0] = -pz * pt * S * Rx + U * px * invPt;
    m_D[i * 2 + 1][i * KFitConst::kNumber7 + 1] = -pz * pt * S * Ry + U * py * invPt;
    if (a != 0.)
      m_D[i * 2 + 1][i * KFitConst::kNumber7 + 2] = -sininv * pt / a;
    else
      m_D[i * 2 + 1][i * KFitConst::kNumber7 + 2] = -a2 * invPt;
    m_D[i * 2 + 1][i * KFitConst::kNumber7 + 4] =  px * pz * pt * S;
    m_D[i * 2 + 1][i * KFitConst::kNumber7 + 5] =  py * pz * pt * S;
    m_D[i * 2 + 1][i * KFitConst::kNumber7 + 6] = -pt;

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
MassVertexFitKFit::calculateNDF(void) {
  m_NDF = 2 * m_TrackCount - 3 + 1;

  return m_ErrorCode = KFitError::kNoError;
}

enum KFitError::ECode MassVertexFitKFit::updateMother(Particle* mother)
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
  CLHEP::HepLorentzVector momClhep = kmm.getMotherMomentum();
  TLorentzVector mom(momClhep.px(), momClhep.py(), momClhep.pz(), momClhep.e());
  CLHEP::Hep3Vector posClhep = kmm.getMotherPosition();
  TVector3 pos(posClhep.x(), posClhep.y(), posClhep.z());
  CLHEP::HepSymMatrix covMatrix = kmm.getMotherError();
  TMatrixFSym errMatrix(7);
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 7; j++) {
      errMatrix[i][j] = covMatrix[i][j];
    }
  }
  double chi2 = getCHIsq();
  int ndf = getNDF();
  double prob = TMath::Prob(chi2, ndf);
  mother->updateMomentum(mom, pos, errMatrix, prob);
  m_ErrorCode = KFitError::kNoError;
  return m_ErrorCode;
}

