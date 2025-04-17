/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include "klm/dataobjects/KLMTrack.h"


using namespace Belle2;

//! Empty constructor for ROOT IO
KLMTrack::KLMTrack():
  RelationsObject(),
  m_Valid(false),
  m_Good(false),
  m_Chi2(0.0),
  m_NumHit(0)
{
  for (int ii = 0 ; ii < 4; ii ++) {
    m_TrackParam[0] = 0.0;
    for (int jj = 0 ; jj < 4; jj ++) {
      m_TrackParamErr[ii][jj] =  0.0;
    }
  }
}

//! Copy constructor
KLMTrack::KLMTrack(const KLMTrack& track) :
  RelationsObject(track),
  m_Valid(track.m_Valid),
  m_Good(track.m_Good),
  m_Chi2(track.m_Chi2),
  m_NumHit(track.m_NumHit)
{

  for (int ii = 0 ; ii < 4; ii ++) {
    m_TrackParam[ii] = track.m_TrackParam[ii];
    for (int jj = 0 ; jj < 4; jj ++) {
      m_TrackParamErr[ii][jj] = track.m_TrackParamErr[ii][jj];
    }
  }
}

//! Assignment operator
KLMTrack& KLMTrack::operator=(const KLMTrack& track)
{
  m_Valid = track.m_Valid;
  m_Good = track.m_Good;
  m_Chi2 = track.m_Chi2;
  m_NumHit = track.m_NumHit;

  for (int ii = 0 ; ii < 4; ii ++) {
    m_TrackParam[ii] = track.m_TrackParam[ii];
    for (int jj = 0 ; jj < 4; jj ++) {
      m_TrackParamErr[ii][jj] = track.m_TrackParamErr[ii][jj];
    }
  }
  return *this;
}

//! Get track parameters in the global system. y = p0 + p1 * x; z = p2 + p3 * x
TVectorD KLMTrack::getTrackParam()
{
  TVectorD trackParam(4);
  trackParam[0] = m_TrackParam[0];
  trackParam[1] = m_TrackParam[1];
  trackParam[2] = m_TrackParam[2];
  trackParam[3] = m_TrackParam[3];

  return trackParam;
}


//! Get invariance matrix of track parameters in the global system.
TMatrixDSym KLMTrack::getTrackParamErr()
{
  TMatrixDSym  trkParamErr(4);
  for (int ii = 0 ; ii < 4; ii ++) {
    for (int jj = 0 ; jj < 4; jj ++) {
      trkParamErr[ii][jj] = m_TrackParamErr[ii][jj];
    }
  }
  return trkParamErr;
}


//! Set track parameters in the global system. y = p0 + p1 * x; z = p2 + p3 * x
void KLMTrack::setTrackParam(const CLHEP::HepVector& trkPar)
{
  m_TrackParam[0] = trkPar[0];
  m_TrackParam[1] = trkPar[1];
  m_TrackParam[2] = trkPar[2];
  m_TrackParam[3] = trkPar[3];

}

//! Set invariance matrix of track parameters in the global system.
void KLMTrack::setTrackParamErr(const CLHEP::HepSymMatrix& trkParErr)
{
  for (int ii = 0 ; ii < 4; ii ++) {
    for (int jj = 0 ; jj < 4; jj ++) {
      m_TrackParamErr[ii][jj] = trkParErr[ii][jj];
    }
  }
}


