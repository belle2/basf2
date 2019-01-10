/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/hepmc/HepMCReader.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <string>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <TLorentzVector.h>


using namespace std;
using namespace Belle2;


void HepMCReader::open(const string& filename)
{
  B2INFO("Reading HEPMC inputfile at " << filename);
  m_input.open(filename.c_str());
  if (!m_input) { throw (HepMCCouldNotOpenFileError() << filename); }
}


int HepMCReader::getEvent(MCParticleGraph& graph, double& eventWeight)
{
  int eventID = -1;

  HepMC::GenEvent evt;
  bool go_on = readNextEvent(evt);
  eventID = evt.event_number();
  eventWeight = 1; // why is evt.weights() a std::vector?
  const int nparticles = evt.particles_size();

  B2DEBUG(10, "Found eventID " << eventID << " with " << nparticles << " particles.");
  evt.print(); // TODO delete me
  if (nparticles <= 0) {
    throw (HepMCEmptyEventError() << nparticles);
  }
  //TODO fill graph
  return eventID;
}

bool HepMCReader::readNextEvent(HepMC::GenEvent& evt)
{
  // walk through the IOStream
  if (m_input) {
    evt.read(m_input);
    if (evt.is_valid()) {
      B2DEBUG(10, "Found valid event.");
      return true;
    } else {
      B2DEBUG(10, "The next event was invalid.");
    }
  }
  B2DEBUG(10, "Could not skip to next event.");
  return false;
}

//===================================================================
//                  Protected methods
//===================================================================


//int HepMCReader::readEventHeader(int& eventID, double& eventWeight)
//{
//  //Get number of particles from file
//  int nparticles = 0;
//  return nparticles;
//}


