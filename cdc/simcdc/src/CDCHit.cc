/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/simcdc/CDCHit.h>
#include <cdc/hitcdc/SimHitCDC.h>

//DataStore classes
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreArray.h>

//Geant4 classes
#include "G4VVisManager.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4Polyline.hh"

//Basic C
#include <assert.h>

//ROOT class
#include <TVector3.h>

namespace Belle2 {

  G4Allocator<CDCHit> CDCHitAllocator;

//------------------
// Copy constructor
//------------------
  CDCHit::CDCHit(const CDCHit &right): B4VHit()
  {
    _trackID = right._trackID;
    _PDG = right._PDG;
    _layer = right._layer;
    _wire = right._wire;
    _driftLength = right._driftLength;
    _globalTime = right._globalTime;
    _edep = right._edep;
    _stepLength = right._stepLength;
    _momentum = right._momentum;
    _posw = right._posw;
    _posIn = right._posIn;
    _posOut = right._posOut;
    _posFlag = right._posFlag;
  }

//-------------
// operator =
//-------------
  const CDCHit& CDCHit::operator=(const CDCHit &right)
  {
    _trackID = right._trackID;
    _PDG = right._PDG;
    _layer = right._layer;
    _wire = right._wire;
    _driftLength = right._driftLength;
    _globalTime = right._globalTime;
    _edep = right._edep;
    _stepLength = right._stepLength;
    _momentum = right._momentum;
    _posw = right._posw;
    _posIn = right._posIn;
    _posOut = right._posOut;
    _posFlag = right._posFlag;
    return *this;
  }

//--------------
// operator ==
//--------------
  int CDCHit::operator==(const CDCHit &right) const
  {
    return ((_trackID == right._trackID) &&
            (_PDG == right._PDG) &&
            (_layer == right._layer) &&
            (_wire == right._wire) &&
            (_driftLength == right._driftLength) &&
            (_globalTime == right._globalTime) &&
            (_edep == right._edep) &&
            (_stepLength == right._stepLength) &&
            (_momentum == right._momentum) &&
            (_posw == right._posw) &&
            (_posIn == right._posIn) &&
            (_posOut == right._posOut) &&
            (_posFlag == right._posFlag));
  }

//------------
// Draw hits
//------------
  void CDCHit::Draw()
  {
    G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
    if (pVVisManager) {
      G4Colour colour(((GetTrackID() % 2) + 1.) / 2., ((GetTrackID() % 3) + 1.) / 3., 1.);
      G4VisAttributes attribs(colour);
      G4Circle circle;
      circle.SetPosition(G4Point3D(GetPosInx(), GetPosIny(), GetPosInz()));
      circle.SetScreenDiameter(2.0);
      circle.SetFillStyle(G4Circle::filled);
      circle.SetVisAttributes(attribs);
      pVVisManager->Draw(circle);
    }
  }

//--------
// Print
//--------
  void CDCHit::Print()
  {
  }

//---------------------------
// Save hits into ASCII file
//---------------------------
  void CDCHit::Save(FILE *oFile)
  {
    if (oFile) {
      fprintf(oFile, "nL:%d nW:%d trkID:%d PDG:%d DriftLen[mm]:%7.2f gTime[ns]:%7.2f edep[keV]%15e stepLen[mm]:%7.2f vecP[GeV]:%7.2f %7.2f %7.2f vecPosw[mm]:%7.2f %7.2f %7.2f vecPosIn[mm]:%7.2f %7.2f %7.2f vecPosOut[mm]:%7.2f %7.2f %7.2f posFlag:%d\n",
              _layer,
              _wire,
              _trackID,
              _PDG,
              _driftLength / cm,
              _globalTime / ns,
              _edep / GeV,
              _stepLength / cm,
              _momentum.getX() / GeV,
              _momentum.getY() / GeV,
              _momentum.getZ() / GeV,
              _posw.getX() / cm,
              _posw.getY() / cm,
              _posw.getZ() / cm,
              _posIn.getX() / cm,
              _posIn.getY() / cm,
              _posIn.getZ() / cm,
              _posOut.getX() / cm,
              _posOut.getY() / cm,
              _posOut.getZ() / cm,
              _posFlag
             );
    }
  }

//---------------------------
// Save hits into DataStore
//---------------------------
  void CDCHit::Save(int ihit)
  {
    StoreArray<SimHitCDC> cdcArray("SimHitCDCArray");
    new(cdcArray->AddrAt(ihit)) SimHitCDC();
    cdcArray[ihit]->setLayerId(_layer);
    cdcArray[ihit]->setWireId(_wire);
    cdcArray[ihit]->setTrackId(_trackID);
    cdcArray[ihit]->setPDGCode(_PDG);
    cdcArray[ihit]->setDriftLength(_driftLength / cm);
    cdcArray[ihit]->setFlightTime(_globalTime / ns);
    cdcArray[ihit]->setEnergyDep(_edep / GeV);
    cdcArray[ihit]->setStepLength(_stepLength / cm);
    TVector3 momentum(_momentum.getX() / GeV, _momentum.getY() / GeV, _momentum.getZ() / GeV);
    cdcArray[ihit]->setMomentum(momentum);
    TVector3 posWire(_posw.getX() / cm, _posw.getY() / cm, _posw.getZ() / cm);
    cdcArray[ihit]->setPosWire(posWire);
    TVector3 posIn(_posIn.getX() / cm, _posIn.getY() / cm, _posIn.getZ() / cm);
    cdcArray[ihit]->setPosIn(posIn);
    TVector3 posOut(_posOut.getX() / cm, _posOut.getY() / cm, _posOut.getZ() / cm);
    cdcArray[ihit]->setPosOut(posOut);
    cdcArray[ihit]->setPosFlag(_posFlag);
  }

//-----------------------------
// Load hits from a text file
//-----------------------------
  G4bool CDCHit::Load(FILE *iFile)
  {
    G4double Px, Py, Pz, PosWx, PosWy, PosWz, PosInx, PosIny, PosInz, PosOutx, PosOuty, PosOutz;
    G4bool ReadStatus;
    ReadStatus =
      (fscanf(iFile, "%d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %d",
              &_layer, &_wire, &_trackID, &_PDG, &_driftLength, &_globalTime, &_edep, &_stepLength, &Px, &Py, &Pz, &PosWx, &PosWy, &PosWz,
              &PosInx, &PosIny, &PosInz, &PosOutx, &PosOuty, &PosOutz, &_posFlag) == 21);

    _driftLength = _driftLength * cm;
    _globalTime = _globalTime * ns;
    _edep = _edep * GeV;
    _stepLength = _stepLength * cm;

    _momentum.setX(Px*GeV);
    _momentum.setY(Py*GeV);
    _momentum.setZ(Pz*GeV);

    _posw.setX(PosWx*cm);
    _posw.setY(PosWy*cm);
    _posw.setZ(PosWz*cm);

    _posIn.setX(PosInx*cm);
    _posIn.setY(PosIny*cm);
    _posIn.setZ(PosInz*cm);

    _posOut.setX(PosOutx*cm);
    _posOut.setY(PosOuty*cm);
    _posOut.setZ(PosOutz*cm);

    return ReadStatus;
  }

}
