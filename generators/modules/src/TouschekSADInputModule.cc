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
#include <generators/dataobjects/MCParticleGraph.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/core/utilities.h>

#include <framework/gearbox/Unit.h>

#include <framework/datastore/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;

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
  addParam("ReadoutTime",     m_readoutTime,   "The readout time of the detector [ns]", 20 * Unit::us);
  addParam("ReadMode",        m_readMode,      "The read mode: 0 = one SAD particle per event, 1 = one real particle per event, 2 = all SAD particles per event", 0);
  addParam("FilenameLER",     m_filenameLER,   "The filename of the LER SAD input file.");
  addParam("RangeLER",        m_rangeLER,      "All particles within the range around the IP are loaded [cm].", 300.0 * Unit::cm);
  addParam("BeamEnergyLER",   m_beamEnergyLER, "The beam energy of the LER [GeV].", 4.0 * Unit::GeV);
  addParam("CurrentLER",      m_currentLER,    "The current of the LER [A].", 3.6);
  addParam("LifetimeLER",     m_lifetimeLER,   "The Touschek lifetime of the LER [ns].", 600 * Unit::s);
  addParam("PxResolutionLER", m_pxResLER,      "The resolution for the x momentum component of the Touschek real particle.", 0.01);
  addParam("PyResolutionLER", m_pyResLER,      "The resolution for the y momentum component of the Touschek real particle.", 0.01);
}


TouschekSADInputModule::~TouschekSADInputModule()
{
}


void TouschekSADInputModule::initialize()
{
  //Register collections
  StoreArray<MCParticle> MCParticles(DEFAULT_MCPARTICLES);

  //Check parameters
  if (!FileSystem::fileExists(m_filenameLER)) {
    B2ERROR("Parameter <FilenameLER>: Could not open the file. The filename " << m_filenameLER << " does not exist !")
  } else m_readerLER.open(m_filenameLER);

  //Set the transformation from local Touschek plane space to global geant4 space
  //Get the parameters from the Gearbox
  m_lerPipePartMatrix = new TGeoHMatrix("TouschekSADTrafoLER");
  GearDir irDir = Gearbox::getInstance().getContent("IR");
  double angle = irDir.getAngle("Streams/Stream[@name='LERUpstream']/Section[@name='Crotch']/Pipe[1]/Angle");
  m_lerPipePartMatrix->RotateY(angle / Unit::deg);

  //-----------------------
  // Initialize LER reader
  //-----------------------
  m_readerLER.initialize(m_lerPipePartMatrix, m_rangeLER, -11, m_beamEnergyLER, m_currentLER, m_lifetimeLER, m_readoutTime);
  m_readerLER.setMomentumRes(m_pxResLER, m_pyResLER);
}


void TouschekSADInputModule::event()
{
  try {
    MCParticleGraph mpg;
    StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

    try {
      //----------------------------------
      //       Read the LER data
      //----------------------------------
      switch (m_readMode) {
        case 0:  readSADParticle(m_readerLER, mpg);
          break;
        case 1:  readRealParticle(m_readerLER, mpg);
          break;
        case 2:  m_readerLER.addAllSADParticles(mpg);
          break;
        default: readSADParticle(m_readerLER, mpg);
          break;
      }

      //----------------------------------
      // Generate MCParticles collection
      //----------------------------------
      mpg.generateList(DEFAULT_MCPARTICLES, MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

    } catch (TouschekReaderSAD::TouschekEndOfFile& exc) {
      B2DEBUG(10, exc.what())
      eventMetaDataPtr->setEndOfData();
      return;
    }
  } catch (runtime_error &exc) {
    B2ERROR(exc.what());
  }
}


//====================================================================
//                       Private methods
//====================================================================

void TouschekSADInputModule::readSADParticle(TouschekReaderSAD& reader, MCParticleGraph& mpg)
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  double weight = reader.getSADParticle(mpg);
  if (weight < 0) return;
  eventMetaDataPtr->setGeneratedWeight(weight);
}


void TouschekSADInputModule::readRealParticle(TouschekReaderSAD& reader, MCParticleGraph& mpg)
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  double weight = reader.getRealParticle(mpg);
  if (weight < 0) return;
  eventMetaDataPtr->setGeneratedWeight(weight);
}
