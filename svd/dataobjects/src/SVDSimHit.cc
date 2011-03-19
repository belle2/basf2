/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// basf2
#include <svd/dataobjects/SVDSimHit.h>

using namespace std;
using namespace Belle2;

ClassImp(SVDSimHit)

SVDSimHit& SVDSimHit::operator=(const SVDSimHit & other)
{
  this->m_layerID = other.getLayerID();
  this->m_ladderID = other.getLadderID();
  this->m_sensorID = other.getSensorID();
  this->m_posIn = other.getPosIn();
  this->m_posOut = other.getPosOut();
  this->m_theta = other.getTheta();
  this->m_momIn = other.getMomIn();
  this->m_PDGcode = other.getPDGcode();
  this->m_PDGmass = other.getPDGmass();
  this->m_PDGcharge = other.getPDGcharge();
  this->m_trackID = other.getTrackID();
  this->m_energyDep = other.getEnergyDep();
  this->m_stepLength = other.getStepLength();
  this->m_globalTime = other.getGlobalTime();
  return *this;
}
