/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "bklm/dataobjects/BKLMTrack.h"
#include <framework/logging/Logger.h>

using namespace Belle2;

//! Empty constructor for ROOT IO
BKLMTrack::BKLMTrack():
  RelationsObject(),
  m_Valid(false),
  m_Good(false),
  m_Chi2(0.0),
  m_NumHit(0)
{
  for (int ii = 0 ; ii < 4; ii ++) {
    m_TrackParam[0] = 0.0;
    m_LocalTrackParam[ii] = 0.0;
    for (int jj = 0 ; jj < 4; jj ++) {
      m_TrackParamErr[ii][jj] =  0.0;
      m_LocalTrackParamErr[ii][jj] = 0.0;
    }
  }
}

//! Copy constructor
BKLMTrack::BKLMTrack(const BKLMTrack& track) :
  RelationsObject(track),
  m_Valid(track.m_Valid),
  m_Good(track.m_Good),
  m_Chi2(track.m_Chi2),
  m_NumHit(track.m_NumHit)
{

  for (int ii = 0 ; ii < 4; ii ++) {
    m_TrackParam[ii] = track.m_TrackParam[ii];
    m_LocalTrackParam[ii] = track.m_LocalTrackParam[ii];
    for (int jj = 0 ; jj < 4; jj ++) {
      m_TrackParamErr[ii][jj] = track.m_TrackParamErr[ii][jj];
      m_LocalTrackParamErr[ii][jj] = track.m_LocalTrackParamErr[ii][jj];
    }
  }
}

//! Assignment operator
BKLMTrack& BKLMTrack::operator=(const BKLMTrack& track)
{
  m_Valid = track.m_Valid;
  m_Good = track.m_Good;
  m_Chi2 = track.m_Chi2;
  m_NumHit = track.m_NumHit;

  for (int ii = 0 ; ii < 4; ii ++) {
    m_TrackParam[ii] = track.m_TrackParam[ii];
    m_LocalTrackParam[ii] = track.m_LocalTrackParam[ii];
    for (int jj = 0 ; jj < 4; jj ++) {
      m_TrackParamErr[ii][jj] = track.m_TrackParamErr[ii][jj];
      m_LocalTrackParamErr[ii][jj] = track.m_LocalTrackParamErr[ii][jj];
    }
  }
  return *this;
}

//! Get track parameters in the global system. y = p0 + p1 * x; z = p2 + p3 * x
TVectorD BKLMTrack::getTrackParam()
{
  TVectorD trackParam(4);
  trackParam[0] = m_TrackParam[0];
  trackParam[1] = m_TrackParam[1];
  trackParam[2] = m_TrackParam[2];
  trackParam[3] = m_TrackParam[3];

  return trackParam;
}


//! Get track parameters in the sector locan system, where the first layer of the sector is used as reference.
TVectorD BKLMTrack::getLocalTrackParam()
{
  TVectorD trackParam(4);
  trackParam[0] = m_LocalTrackParam[0];
  trackParam[1] = m_LocalTrackParam[1];
  trackParam[2] = m_LocalTrackParam[2];
  trackParam[3] = m_LocalTrackParam[3];

  return trackParam;
}

//! Get invariance matrix of track parameters in the global system.
TMatrixDSym BKLMTrack::getTrackParamErr()
{
  TMatrixDSym  trkParamErr(4);
  for (int ii = 0 ; ii < 4; ii ++) {
    for (int jj = 0 ; jj < 4; jj ++) {
      trkParamErr[ii][jj] = m_TrackParamErr[ii][jj];
    }
  }
  return trkParamErr;
}

//! Get invariance matrix of track parameters in the sector local system, where the first layer of the sector is used as reference.
TMatrixDSym BKLMTrack::getLocalTrackParamErr()
{
  TMatrixDSym  trkParamErr(4);
  for (int ii = 0 ; ii < 4; ii ++) {
    for (int jj = 0 ; jj < 4; jj ++) {
      trkParamErr[ii][jj] = m_LocalTrackParamErr[ii][jj];
    }
  }
  return trkParamErr;
}

//! Set track parameters in the global system. y = p0 + p1 * x; z = p2 + p3 * x
void BKLMTrack::setTrackParam(const CLHEP::HepVector& trkPar)
{
  m_TrackParam[0] = trkPar[0];
  m_TrackParam[1] = trkPar[1];
  m_TrackParam[2] = trkPar[2];
  m_TrackParam[3] = trkPar[3];

}

//! Set invariance matrix of track parameters in the global system.
void BKLMTrack::setTrackParamErr(const CLHEP::HepSymMatrix& trkParErr)
{
  for (int ii = 0 ; ii < 4; ii ++) {
    for (int jj = 0 ; jj < 4; jj ++) {
      m_TrackParamErr[ii][jj] = trkParErr[ii][jj];
    }
  }
}

//! Set track parameters in the sector local system, where the first layer of the sector is used as reference.
void BKLMTrack::setLocalTrackParam(const CLHEP::HepVector& trkPar)
{
  m_LocalTrackParam[0] = trkPar[0];
  m_LocalTrackParam[1] = trkPar[1];
  m_LocalTrackParam[2] = trkPar[2];
  m_LocalTrackParam[3] = trkPar[3];

}

//! Set invariance matrix of track parameters in the sector local system, where the first layer of the sector is used as reference.
void BKLMTrack::setLocalTrackParamErr(const CLHEP::HepSymMatrix& trkParErr)
{
  for (int ii = 0 ; ii < 4; ii ++) {
    for (int jj = 0 ; jj < 4; jj ++) {
      m_LocalTrackParamErr[ii][jj] = trkParErr[ii][jj];
    }
  }
}

//! Get the positon in local coordinate system of track intercept in plane of constant x
TVector3 BKLMTrack::getLocalIntercept(double x)
{

  TVector3 intercept;

  intercept[0] =  x ;
  intercept[1] =  m_LocalTrackParam[ 0 ] + x * m_LocalTrackParam[ 1 ] ;
  intercept[2] =  m_LocalTrackParam[ 2 ] + x * m_LocalTrackParam[ 3 ] ;

  return intercept;

}

//! Get the variance matrix of (y,z) coordinates of the track intercept in plane of constant x in sector local system
TMatrixD BKLMTrack::getLocalInterceptVariance(double x)
{

  TMatrixD  errors(2, 2, 0);
  TMatrixD  A(2, 4, 0);
  A[ 0 ][ 0 ] = 1.0;
  A[ 0 ][ 1 ] =   x;
  A[ 1 ][ 2 ] = 1.0;
  A[ 1 ][ 3 ] =   x;
  errors = A * getLocalTrackParamErr() * A.T();

  return errors;

}
