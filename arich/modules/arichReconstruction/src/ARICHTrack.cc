/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik, Dino Tahirovic              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/modules/arichReconstruction/ARICHTrack.h>
#include <arich/geometry/ARICHGeometryPar.h>
#include <cmath>

using namespace std;
using namespace Belle2;

ARICHTrack::ARICHTrack(const ARICHAeroHit& aeroHit)
{

  m_originalPosition = aeroHit.getPosition();
  m_originalDirection = aeroHit.getMomentum().Unit();
  m_originalMomentum =  aeroHit.getMomentum().Mag();
  m_reconstructedPosition = TVector3(0, 0, 0);
  m_reconstructedDirection = TVector3(0, 0, 0);
  m_reconstructedMomentum = -1.0;
  m_PDGCharge = 0; // check if aeroHit contains this info
  m_PDGEncoding = aeroHit.getPDG();
  m_trackID = aeroHit.getTrackID();
  m_extHitID = -1;
  m_identity = Lund2Type(m_PDGEncoding);
  m_aeroIndex = -1;
  m_detectedPhotons = -1;
  for (int i = 0; i < c_noOfHypotheses; i++) {
    m_lkh[i] = 0;
    m_expectedPhotons[i] = 0;
    m_acc[i] = 0;
  }
}

ARICHTrack::ARICHTrack(const ExtHit* extHit, int charge, int pdgCode, int trackID, int aeroHitIndex) :
  m_originalPosition(0, 0, 0),
  m_originalDirection(0, 0, 0),
  m_originalMomentum(-1),
  m_reconstructedPosition(extHit->getPosition()),
  m_reconstructedDirection(extHit->getMomentum().Unit()),
  m_reconstructedMomentum(extHit->getMomentum().Mag()),
  m_PDGCharge(charge),
  m_PDGEncoding(pdgCode),
  m_trackID(trackID),
  m_extHitID(extHit->getArrayIndex()),
  m_identity(Lund2Type(m_PDGEncoding)),
  m_aeroIndex(aeroHitIndex),
  m_detectedPhotons(-1)
{
  for (int i = 0; i < c_noOfHypotheses; i++) {
    m_lkh[i] = 0;
    m_expectedPhotons[i] = 0;
    m_acc[i] = 0;
  }
}

int ARICHTrack::Lund2Type(int ipart)
{
  //Description: Converts GEANT code to e mu pi K p index.
  // return =0  electron
  //        =1  muon
  //        =2  pion
  //        =3  kaon
  //        =4  proton
  //        =-1   ?
  ipart = abs(ipart);
  switch (ipart) {
    case  11  :  return 0;
    case  13  :  return 1;
    case  211 :  return 2;
    case  321 :  return 3;
    case  2212:  return 4;
    default:     return -1;
  }
}

double ARICHTrack::getMeanEmissionLength(int i) const
{
  // Emission length measured from aerogel exit
  static arich::ARICHGeometryPar* arichgp = arich::ARICHGeometryPar::Instance();

  TVector3 dir = getMeanEmissionDirection(i);
  if (dir.Z() == 0) return 0;
  double atl = arichgp->getAerogelTransmissionLength(i);
  double d   = arichgp->getAerogelThickness(i) / dir.Z() / atl;
  double dmean = 1 - d / (exp(d) - 1);
  return (dmean * atl);
}

TVector3 ARICHTrack::getMeanEmissionPosition(int i) const
{

  TVector3 dir = getMeanEmissionDirection(i);
  if (dir.Z() == 0) return TVector3();
  return (getAerogelExit(i) - getMeanEmissionLength(i) * dir);
}


const TVector3 ARICHTrack::getAerogelExit(int i) const
{

  static arich::ARICHGeometryPar* arichgp = arich::ARICHGeometryPar::Instance();
  double z = arichgp->getAerogelZPosition(i) + arichgp->getAerogelThickness(i);
  return  getPositionAtZ(z);
}

const TVector3 ARICHTrack::getAerogelInput(int i) const
{

  static arich::ARICHGeometryPar* arichgp = arich::ARICHGeometryPar::Instance();
  double z = arichgp->getAerogelZPosition(i);
  return  getPositionAtZ(z);
}

const TVector3 ARICHTrack::getPositionAtZ(double zout) const
{

  if (m_reconstructedDirection.Z() == 0) return m_reconstructedPosition;
  double path = (zout - m_reconstructedPosition.Z()) / m_reconstructedDirection.Z();
  return m_reconstructedPosition + m_reconstructedDirection * path;
}



//} // namespace arich
//} // namespace Belle2
