/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// basf2
#include <pxd/hitpxd/PXDSimHit.h>
#include <pxd/simpxd/PXDB4VHit.h>
#include <framework/logging/Logger.h>

// root
#include <TString.h>

// Geant
#include "G4ThreeVector.hh"
#include "G4String.hh"

using namespace std;
using namespace Belle2;

ClassImp(PXDSimHit)

PXDSimHit::PXDSimHit(const PXDB4VHit & g4hit):
    m_posIn(g4hit.getPosIn().x(), g4hit.getPosIn().y(), g4hit.getPosIn().z()),
    m_posOut(g4hit.getPosOut().x(), g4hit.getPosOut().y(), g4hit.getPosOut().z()),
    m_theta(g4hit.getTheta()),
    m_momIn(g4hit.getMomIn().x(), g4hit.getMomIn().y(), g4hit.getMomIn().z()),
    m_PDGcode(g4hit.getPDGcode()),
    m_trackID(g4hit.getTrackID()),
    m_energyDep(g4hit.getEnergyDep()),
    m_stepLength(g4hit.getStepLength()),
    m_globalTime(g4hit.getGlobalTime())
{
  /* Parse g4hit.volumeName for layer, ladder and sensor numbers */
  TString vname(g4hit.getVolumeName().data());
  m_layerID = TString(vname(vname.Index("Layer_") + 6)).Atoi();
  m_ladderID = TString(vname(vname.Index("Ladder_") + 7)).Atoi();
  m_sensorID = TString(vname(vname.Index("Sensor_") + 7)).Atoi();
}

PXDSimHit& PXDSimHit::operator=(const PXDSimHit & other)
{
  this->m_layerID = other.getLayerID();
  this->m_ladderID = other.getLadderID();
  this->m_sensorID = other.getSensorID();
  this->m_posIn = other.getPosIn();
  this->m_posOut = other.getPosOut();
  this->m_theta = other.getTheta();
  this->m_momIn = other.getMomIn();
  this->m_PDGcode = other.getPDGcode();
  this->m_trackID = other.getTrackID();
  this->m_energyDep = other.getEnergyDep();
  this->m_stepLength = other.getStepLength();
  this->m_globalTime = other.getGlobalTime();
  return *this;
}


