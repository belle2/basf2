/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/TouschekSADInputModule.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/core/ModuleUtils.h>

#include <framework/gearbox/Unit.h>

using namespace std;
using namespace Belle2;
using namespace Generators;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TouschekSADInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TouschekSADInputModule::TouschekSADInputModule() : Module()
{
  //Set module properties
  setDescription("Reads the Touschek data from a SAD file and stores it into the MCParticle collection.");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("FilenameLER",  m_filenameLER, "The filename of the LER SAD input file.");
  addParam("MaxParticles", m_maxParticles, "The maximum number of particles per event that should be read. -1 means all particles are read.", 10);
  addParam("RangeLER",  m_RangeLER, "All particles within the range around the IP are loaded [cm].", 300.0);

  //Create and initialize member variables
  m_lerPipePartMatrix = new TGeoHMatrix("TouschekSADTrafoLER");
  m_readerLER = new TouschekReaderSAD(m_lerPipePartMatrix); //LER: positrons
}


TouschekSADInputModule::~TouschekSADInputModule()
{
  delete m_readerLER;
}


void TouschekSADInputModule::initialize()
{
  //Check parameters
  if (!ModuleUtils::fileNameExists(m_filenameLER)) {
    B2ERROR("Parameter <FilenameLER>: Could not open the file. The filename " << m_filenameLER << " does not exist !")
  } else m_readerLER->open(m_filenameLER);

  //Set the transformation from local Touschek plane space to global geant4 space
  //Get the parameters from the Gearbox
  GearDir irDir = Gearbox::Instance().getContent("IR");
  double angle1 = irDir.getParamAngle("Streams/Stream[@name='LERUpstream']/Section[@name='Inner']/Pipe[1]/Angle");
  double angle2 = irDir.getParamAngle("Streams/Stream[@name='LERUpstream']/Section[@name='Inner']/Pipe[2]/Angle");
  m_lerPipePartMatrix->RotateY((angle1 + angle2) / Unit::deg);
}


void TouschekSADInputModule::event()
{
  try {
    //Read the data
    MCParticleGraph mpg;
    int readLERParticles = m_readerLER->getParticles(m_maxParticles, m_RangeLER * Unit::cm, 4.0 * Unit::GeV, -11, mpg); //LER: positrons with 4 GeV (sorry, hardcoded)

    if (readLERParticles > 0) {
      //Generate MCParticle list
      mpg.generateList(DEFAULT_MCPARTICLES, MCParticleGraph::set_decay_info | MCParticleGraph::check_cyclic);

      B2INFO("Read " << readLERParticles << " e+ particles (LER).")
    }
  } catch (runtime_error &exc) {
    B2ERROR(exc.what());
  }
}
