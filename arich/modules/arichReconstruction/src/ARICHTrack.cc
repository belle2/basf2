/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/modules/arichReconstruction/ARICHTrack.h>
#include <arich/geoarich/ARICHGeometryPar.h>
#include <cmath>

using namespace std;
using namespace Belle2;


ARICHTrack::ARICHTrack(const ARICHAeroHit &aeroHit)
{

  _OriginalPosition = aeroHit.getPosition();
  _OriginalDirection = aeroHit.getMomentum();
  _OriginalMomentum =   _OriginalDirection.Mag();
  _OriginalDirection =  _OriginalDirection.Unit();
  _PDGEncoding = aeroHit.getParticleID();
  _G4TrackID = aeroHit.getTrackID();
  _Identity = Lund2Type(_PDGEncoding);

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
  ipart = abs(double(ipart));
  switch (ipart) {
    case  11  :  return 0;
    case  13  :  return 1;
    case  211 :  return 2;
    case  321 :  return 3;
    case  2212:  return 4;
    default:     return -1;
  }
}

double ARICHTrack::GetMeanEmissionLength(int i) const
{
  // Emission length measured from aerogel exit
  static ARICHGeometryPar *arichgp = ARICHGeometryPar::Instance();

  TVector3 dir = GetMeanEmissionDirection(i);
  if (dir.Z() == 0) return 0;
  double atl = arichgp->GetAerogelTransmissionLength(i);
  double d   = arichgp->GetAerogelThickness(i) / dir.Z() / atl;
  double dmean = 1 - d / (exp(d) - 1);
  return (dmean*atl);
}

TVector3 ARICHTrack::GetMeanEmissionPosition(int i) const
{

  TVector3 dir = GetMeanEmissionDirection(i);
  if (dir.Z() == 0) return TVector3();
  return (GetAerogelExit(i) - GetMeanEmissionLength(i)*dir);
}


const TVector3 ARICHTrack::GetAerogelExit(int i) const
{

  static ARICHGeometryPar *arichgp = ARICHGeometryPar::Instance();
  double z = arichgp->GetAerogelZPosition(i) + arichgp->GetAerogelThickness(i);
  return  GetPositionAtZ(z);
}

const TVector3 ARICHTrack::GetAerogelInput(int i) const
{

  static ARICHGeometryPar *arichgp = ARICHGeometryPar::Instance();
  double z = arichgp->GetAerogelZPosition(i);
  return  GetPositionAtZ(z);
}

const TVector3 ARICHTrack::GetPositionAtZ(double zout) const
{

  if (_ReconstructedDirection.Z() == 0) return _ReconstructedPosition;
  double path = (zout - _ReconstructedPosition.Z()) / _ReconstructedDirection.Z();
  return _ReconstructedPosition + _ReconstructedDirection*path;
}


ClassImp(ARICHTrack)



