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
  m_input.open(filename.c_str());
  if (!m_input) { throw (HepMCCouldNotOpenFileError() << filename); }
}


int HepMCReader::getEvent(MCParticleGraph& graph, double& eventWeight)
{
  int eventID = -1;
  int nparticles = 0;
  if (nparticles <= 0) {
    throw (HepMCEmptyEventError() << nparticles);
  }

  return eventID;
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


