/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <cstdio>

#include <tracking/kinkFinding/fitter/kinkVertexFitterKFitTrack.h>

using namespace CLHEP;
using namespace std;
using namespace Belle2;


kinkVertexFitterKFitTrack::kinkVertexFitterKFitTrack()
{
  this->m_PXEBefore.m_P  = HepLorentzVector();
  this->m_PXEBefore.m_X  = HepPoint3D();
  this->m_PXEBefore.m_E  = HepSymMatrix(kinkVertexFitterKFitConst::kNumber7, 0);
  this->m_PXEAfter. m_P  = HepLorentzVector();
  this->m_PXEAfter. m_X  = HepPoint3D();
  this->m_PXEAfter. m_E  = HepSymMatrix(kinkVertexFitterKFitConst::kNumber7, 0);
  this->m_Charge         = 0.;
  this->m_Mass           = 0.;
  this->m_Vertex         = HepPoint3D();
  this->m_VertexError    = HepSymMatrix(3, 0);
}


kinkVertexFitterKFitTrack::kinkVertexFitterKFitTrack(const kinkVertexFitterKFitTrack& a)  = default;


kinkVertexFitterKFitTrack::kinkVertexFitterKFitTrack
(
  const CLHEP::HepLorentzVector& p, const HepPoint3D& x, const CLHEP::HepSymMatrix& e, const double charge,
  const int flag
)
{
  checkFlag(flag);
  checkMatrixDimension(e, kinkVertexFitterKFitConst::kNumber7);

  switch (flag) {
    case kinkVertexFitterKFitConst::kBeforeFit:
      m_PXEBefore.m_P = p;
      m_PXEBefore.m_X = x;
      m_PXEBefore.m_E = e;
      m_PXEAfter.m_P  = HepLorentzVector();
      m_PXEAfter.m_X  = HepPoint3D();
      m_PXEAfter.m_E  = HepSymMatrix(kinkVertexFitterKFitConst::kNumber7, 0);
      break;

    case kinkVertexFitterKFitConst::kAfterFit:
      m_PXEBefore.m_P = HepLorentzVector();
      m_PXEBefore.m_X = HepPoint3D();
      m_PXEBefore.m_E = HepSymMatrix(kinkVertexFitterKFitConst::kNumber7, 0);
      m_PXEAfter.m_P  = p;
      m_PXEAfter.m_X  = x;
      m_PXEAfter.m_E  = e;
      break;
  }

  m_Charge      = charge;
  m_Mass        = p.mag();
  m_Vertex      = HepPoint3D();
  m_VertexError = HepSymMatrix(3, 0);
}


kinkVertexFitterKFitTrack::~kinkVertexFitterKFitTrack() = default;


kinkVertexFitterKFitTrack&
kinkVertexFitterKFitTrack::operator = (const kinkVertexFitterKFitTrack& a)
{
  if (this != &a) {
    this->m_PXEBefore   = a.m_PXEBefore;
    this->m_PXEAfter    = a.m_PXEAfter;
    this->m_Charge      = a.m_Charge;
    this->m_Mass        = a.m_Mass;
    this->m_Vertex      = a.m_Vertex;
    this->m_VertexError = a.m_VertexError;
  }

  return *this;
}


void
kinkVertexFitterKFitTrack::setMomentum(const HepLorentzVector& p, const int flag)
{
  checkFlag(flag);

  if (flag == kinkVertexFitterKFitConst::kBeforeFit)
    m_PXEBefore.m_P = p;
  else
    m_PXEAfter.m_P = p;

  m_Mass = p.mag();
}


void
kinkVertexFitterKFitTrack::setPosition(const HepPoint3D& x, const int flag)
{
  checkFlag(flag);

  if (flag == kinkVertexFitterKFitConst::kBeforeFit)
    m_PXEBefore.m_X = x;
  else
    m_PXEAfter.m_X = x;
}


void
kinkVertexFitterKFitTrack::setError(const HepSymMatrix& e, const int flag)
{
  checkFlag(flag);
  checkMatrixDimension(e, kinkVertexFitterKFitConst::kNumber7);

  if (flag == kinkVertexFitterKFitConst::kBeforeFit)
    m_PXEBefore.m_E = e;
  else
    m_PXEAfter.m_E = e;
}


void
kinkVertexFitterKFitTrack::setCharge(const double charge)
{
  m_Charge = charge;
}


void
kinkVertexFitterKFitTrack::setVertex(const HepPoint3D& v)
{
  m_Vertex = v;
}


void
kinkVertexFitterKFitTrack::setVertexError(const HepSymMatrix& ve)
{
  checkMatrixDimension(ve, 3);
  m_VertexError = ve;
}


const HepLorentzVector
kinkVertexFitterKFitTrack::getMomentum(const int flag) const
{
  checkFlag(flag);
  return flag == kinkVertexFitterKFitConst::kBeforeFit ? m_PXEBefore.m_P : m_PXEAfter.m_P;
}


const HepPoint3D
kinkVertexFitterKFitTrack::getPosition(const int flag) const
{
  checkFlag(flag);
  return flag == kinkVertexFitterKFitConst::kBeforeFit ? m_PXEBefore.m_X : m_PXEAfter.m_X;
}


const HepSymMatrix
kinkVertexFitterKFitTrack::getError(const int flag) const
{
  checkFlag(flag);
  return flag == kinkVertexFitterKFitConst::kBeforeFit ? m_PXEBefore.m_E : m_PXEAfter.m_E;
}


double
kinkVertexFitterKFitTrack::getCharge() const
{
  return m_Charge;
}


double
kinkVertexFitterKFitTrack::getMass() const
{
  return m_Mass;
}


const HepPoint3D
kinkVertexFitterKFitTrack::getVertex() const
{
  return m_Vertex;
}


const HepSymMatrix
kinkVertexFitterKFitTrack::getVertexError() const
{
  return m_VertexError;
}


double
kinkVertexFitterKFitTrack::getFitParameter(const int which, const int flag) const
{
  checkFlag(flag);

  const struct KFitPXE& pxe = flag == kinkVertexFitterKFitConst::kBeforeFit ? m_PXEBefore : m_PXEAfter;

  switch (which) {
    case 0: return pxe.m_P.x();
    case 1: return pxe.m_P.y();
    case 2: return pxe.m_P.z();
    case 3: return pxe.m_X.x();
    case 4: return pxe.m_X.y();
    case 5: return pxe.m_X.z();
    default: {
      char buf[1024];
      sprintf(buf, "%s:%s(): which=%d out of range", __FILE__, __func__, which);
      B2FATAL(buf);
    }
  }

  /* NEVER REACHED */
  return -999.;
}


const HepMatrix
kinkVertexFitterKFitTrack::getFitParameter(const int flag) const
{
  HepMatrix a(kinkVertexFitterKFitConst::kNumber6, 1, 0);

  for (int i = 0; i <= 5; i++)
    a[i][0] = getFitParameter(i, flag);

  return a;
}


const HepSymMatrix
kinkVertexFitterKFitTrack::getFitError(const int flag) const
{
  checkFlag(flag);
  HepSymMatrix err(kinkVertexFitterKFitConst::kNumber6, 0);

  const HepSymMatrix& e = flag == kinkVertexFitterKFitConst::kBeforeFit ? m_PXEBefore.m_E : m_PXEAfter.m_E;


  for (int i = 0; i < 3; i++) {
    for (int j = i; j < 3; j++) {
      err[i][j]     = e[i][j];
      err[3 + i][3 + j] = e[4 + i][4 + j];
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      err[i][3 + j]   = e[i][4 + j];
    }
  }

  return err;
}


const HepMatrix
kinkVertexFitterKFitTrack::getMomPos(const int flag) const
{
  HepMatrix a(kinkVertexFitterKFitConst::kNumber7, 1, 0);

  switch (flag) {
    case kinkVertexFitterKFitConst::kBeforeFit:
      a[0][0] = m_PXEBefore.m_P.x();
      a[1][0] = m_PXEBefore.m_P.y();
      a[2][0] = m_PXEBefore.m_P.z();
      a[3][0] = m_PXEBefore.m_P.t();
      a[4][0] = m_PXEBefore.m_X.x();
      a[5][0] = m_PXEBefore.m_X.y();
      a[6][0] = m_PXEBefore.m_X.z();
      break;

    case kinkVertexFitterKFitConst::kAfterFit:
      a[0][0] = m_PXEAfter.m_P.x();
      a[1][0] = m_PXEAfter.m_P.y();
      a[2][0] = m_PXEAfter.m_P.z();
      a[3][0] = m_PXEAfter.m_P.t();
      a[4][0] = m_PXEAfter.m_X.x();
      a[5][0] = m_PXEAfter.m_X.y();
      a[6][0] = m_PXEAfter.m_X.z();
  }

  return a;
}
