/**************************************************************************
 * Copyright(C) 1997 - J. Tanaka                                          *
 *                                                                        *
 * Author: J. Tanaka                                                      *
 * Contributor: J. Tanaka and                                             *
 *              conversion to Belle II structure by T. Higuchi            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <stdio.h>

#include <analysis/KFit/KFitTrack.h>

using namespace CLHEP;
using namespace std;
using namespace Belle2;
using namespace Belle2::analysis;


KFitTrack::KFitTrack(void)
{
  this->m_PXEBefore.m_P  = HepLorentzVector();
  this->m_PXEBefore.m_X  = HepPoint3D();
  this->m_PXEBefore.m_E  = HepSymMatrix(KFitConst::kNumber7, 0);
  this->m_PXEAfter. m_P  = HepLorentzVector();
  this->m_PXEAfter. m_X  = HepPoint3D();
  this->m_PXEAfter. m_E  = HepSymMatrix(KFitConst::kNumber7, 0);
  this->m_Charge         = 0.;
  this->m_Mass           = 0.;
  this->m_Vertex         = HepPoint3D();
  this->m_VertexError    = HepSymMatrix(3, 0);
}


KFitTrack::KFitTrack(const KFitTrack& a) :
  m_PXEBefore(a.m_PXEBefore),
  m_PXEAfter(a.m_PXEAfter),
  m_Charge(a.m_Charge),
  m_Mass(a.m_Mass),
  m_Vertex(a.m_Vertex),
  m_VertexError(a.m_VertexError)
{
}


KFitTrack::KFitTrack
(
  const HepLorentzVector& p, const HepPoint3D& x, const HepSymMatrix& e, const double charge,
  const int flag
)
{
  checkFlag(flag);
  checkMatrixDimension(e, KFitConst::kNumber7);

  switch (flag) {
    case KFitConst::kBeforeFit:
      m_PXEBefore.m_P = p;
      m_PXEBefore.m_X = x;
      m_PXEBefore.m_E = e;
      m_PXEAfter.m_P  = HepLorentzVector();
      m_PXEAfter.m_X  = HepPoint3D();
      m_PXEAfter.m_E  = HepSymMatrix(KFitConst::kNumber7, 0);
      break;

    case KFitConst::kAfterFit:
      m_PXEBefore.m_P = HepLorentzVector();
      m_PXEBefore.m_X = HepPoint3D();
      m_PXEBefore.m_E = HepSymMatrix(KFitConst::kNumber7, 0);
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


KFitTrack::~KFitTrack(void)
{
}


KFitTrack&
KFitTrack::operator = (const KFitTrack& a)
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
KFitTrack::setMomentum(const HepLorentzVector& p, const int flag)
{
  checkFlag(flag);

  if (flag == KFitConst::kBeforeFit)
    m_PXEBefore.m_P = p;
  else
    m_PXEAfter.m_P = p;

  m_Mass = p.mag();
}


void
KFitTrack::setPosition(const HepPoint3D& x, const int flag)
{
  checkFlag(flag);

  if (flag == KFitConst::kBeforeFit)
    m_PXEBefore.m_X = x;
  else
    m_PXEAfter.m_X = x;
}


void
KFitTrack::setError(const HepSymMatrix& e, const int flag)
{
  checkFlag(flag);
  checkMatrixDimension(e, KFitConst::kNumber7);

  if (flag == KFitConst::kBeforeFit)
    m_PXEBefore.m_E = e;
  else
    m_PXEAfter.m_E = e;
}


void
KFitTrack::setCharge(const double charge)
{
  m_Charge = charge;
}


void
KFitTrack::setVertex(const HepPoint3D& v)
{
  m_Vertex = v;
}


void
KFitTrack::setVertexError(const HepSymMatrix& ve)
{
  checkMatrixDimension(ve, 3);
  m_VertexError = ve;
}


const HepLorentzVector
KFitTrack::getMomentum(const int flag) const
{
  checkFlag(flag);
  return flag == KFitConst::kBeforeFit ? m_PXEBefore.m_P : m_PXEAfter.m_P;
}


const HepPoint3D
KFitTrack::getPosition(const int flag) const
{
  checkFlag(flag);
  return flag == KFitConst::kBeforeFit ? m_PXEBefore.m_X : m_PXEAfter.m_X;
}


const HepSymMatrix
KFitTrack::getError(const int flag) const
{
  checkFlag(flag);
  return flag == KFitConst::kBeforeFit ? m_PXEBefore.m_E : m_PXEAfter.m_E;
}


double
KFitTrack::getCharge(void) const
{
  return m_Charge;
}


double
KFitTrack::getMass(void) const
{
  return m_Mass;
}


const HepPoint3D
KFitTrack::getVertex(void) const
{
  return m_Vertex;
}


const HepSymMatrix
KFitTrack::getVertexError(void) const
{
  return m_VertexError;
}


double
KFitTrack::getFitParameter(const int which, const int flag) const
{
  checkFlag(flag);

  const struct KFitPXE& pxe = flag == KFitConst::kBeforeFit ? m_PXEBefore : m_PXEAfter;

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
KFitTrack::getFitParameter(const int flag) const
{
  HepMatrix a(KFitConst::kNumber6, 1, 0);

  for (int i = 0; i <= 5; i++)
    a[i][0] = getFitParameter(i, flag);

  return a;
}


const HepSymMatrix
KFitTrack::getFitError(const int flag) const
{
  checkFlag(flag);
  HepSymMatrix err(KFitConst::kNumber6, 0);

  const HepSymMatrix& e = flag == KFitConst::kBeforeFit ? m_PXEBefore.m_E : m_PXEAfter.m_E;


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
KFitTrack::getMomPos(const int flag) const
{
  HepMatrix a(KFitConst::kNumber7, 1, 0);

  switch (flag) {
    case KFitConst::kBeforeFit:
      a[0][0] = m_PXEBefore.m_P.x();
      a[1][0] = m_PXEBefore.m_P.y();
      a[2][0] = m_PXEBefore.m_P.z();
      a[3][0] = m_PXEBefore.m_P.t();
      a[4][0] = m_PXEBefore.m_X.x();
      a[5][0] = m_PXEBefore.m_X.y();
      a[6][0] = m_PXEBefore.m_X.z();
      break;

    case KFitConst::kAfterFit:
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
