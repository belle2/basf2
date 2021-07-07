/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/cryinput/CRYInputModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/utilities/FileSystem.h>

#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>  // For Ubuntu Linux

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CRYInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CRYInputModule::CRYInputModule() : Module()
{
  //Set module properties
  setDescription(R"DOC(Generates cosmic showers with CRY.

The showers will be generated in the xz plane in a square plane of n bt n meters
centered at the IP (n can be choosen with the boxLength parameter). Then the
cosmics will be propagated from there to the corner of of a n times n times n
cube centered around the IP.

Finally, if the track would intersect with the acceptance volume the track is
kept. The acceptance volume can be either a sphere, a cylinder or a box centered
at the IP. This depends on how many values are given to the acceptance parameter.
)DOC");
  addParam("CosmicDataDir", m_cosmicdatadir, R"DOC(Directory that holds the cosmic
data for CRY. Empty string will look in the default location)DOC", std::string(""));
  addParam("acceptance", m_acceptance, R"DOC(Size of the acceptance volume.
This can be either be:

1. one value being the radius of a sphere
2. two values for the radius (in xy) and the half-length (in z) of a cylinder
3. three values for x,y,z half-length of a box

All volumes are centered around the IP. All values are in cm)DOC", m_acceptance);
  addParam("maxTrials", m_maxTrials, "Maximum number of trials per event", m_maxTrials);
  addParam("kineticEnergyThreshold", m_kineticEnergyThreshold,
           "Energy threshold [GeV]", m_kineticEnergyThreshold);
  addParam("timeOffset", m_timeOffset, "Time offset [s]", m_timeOffset);
  addParam("boxLength", m_boxLength, R"DOC(Length of the side of the square in
the xz plane in which CRY will generate cosmics in cm. Will be rounded down to
meters. According to the CRY documentation good values are 1, 3, 10, 30, 100 and
300m but anything in between should work as well. The default is 100m and should
be fine for almost all use cases.)DOC", m_boxLength);
  addParam("date", m_date, R"DOC(Date used for the generation: a string in the form of
'month-date-year'. The cosmic-ray distribution is adjusted to account for the eleven
year, sunspot cycle (the default solar minimum date is January 1, 2008).)DOC", m_date);
  addParam("returnGammas", m_returnGammas,
           "Whether or not CRY should return gammas", m_returnGammas);
  addParam("returnKaons", m_returnKaons,
           "Whether or not CRY should return kaons", m_returnKaons);
  addParam("returnPions", m_returnPions,
           "Whether or not CRY should return pions", m_returnPions);
  addParam("returnProtons", m_returnProtons,
           "Whether or not CRY should return protons", m_returnProtons);
  addParam("returnNeutrons", m_returnNeutrons,
           "Whether or not CRY should return neutrons", m_returnNeutrons);
  addParam("returnElectrons", m_returnElectrons,
           "Whether or not CRY should return electrons", m_returnElectrons);
  addParam("returnMuons", m_returnMuons,
           "Whether or not CRY should return muons", m_returnMuons);
}

void CRYInputModule::initialize()
{
  if (m_cosmicdatadir.empty()) {
    m_cosmicdatadir = FileSystem::findFile("data/generators/modules/cryinput/");
  }

  if (m_boxLength < 100 or m_boxLength > 30000) {
    B2FATAL("Box length should be between 100 and 30000 cm (1 to 300 m)");
  }

  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  m_generator.setCosmicDataDir(m_cosmicdatadir);
  m_generator.setAcceptance(m_acceptance);
  m_generator.setMaxTrials(m_maxTrials);
  m_generator.setKineticEnergyThreshold(m_kineticEnergyThreshold);
  m_generator.setTimeOffset(m_timeOffset);
  m_generator.setBoxLength(m_boxLength);
  m_generator.setDate(m_date);
  m_generator.setReturnGammas(m_returnGammas);
  m_generator.setReturnKaons(m_returnKaons);
  m_generator.setReturnPions(m_returnPions);
  m_generator.setReturnProtons(m_returnProtons);
  m_generator.setReturnNeutrons(m_returnNeutrons);
  m_generator.setReturnElectrons(m_returnElectrons);
  m_generator.setReturnMuons(m_returnMuons);
  m_generator.init();
}

void CRYInputModule::event()
{
  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph);
  m_mcGraph.generateList();
}

void CRYInputModule::terminate()
{
  m_generator.term();
}
