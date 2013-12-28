/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/SADInputModule.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <TGeoMatrix.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SADInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SADInputModule::SADInputModule() : Module()
{
  //Set module properties
  setDescription("Reads the SAD data from a root file and stores it into the MCParticle collection.");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("AccRing",        m_accRing,      "The accelerator ring: 0 = LER, 1 = HER");
  addParam("ReadoutTime",    m_readoutTime,  "The readout time of the detector [ns]", 20 * Unit::us);
  addParam("ReadMode",       m_readMode,     "The read mode: 0 = one SAD particle per event, 1 = one real particle per event, 2 = all SAD particles per event", 0);
  addParam("Filename",       m_filename,     "The filename of the SAD input file.");
  addParam("Range",          m_range,        "All particles within the range around the IP are loaded [cm].", 300.0 * Unit::cm);
  addParam("PxResolution",   m_pxRes,        "The resolution for the x momentum component of the SAD real particle.", 0.01);
  addParam("PyResolution",   m_pyRes,        "The resolution for the y momentum component of the SAD real particle.", 0.01);
  addParam("RotateParticles", m_rotateParticles, "Rotate the SAD particles around the nominal beam axis [deg] (just for unphysical tests !!!).", 0.0);

}


void SADInputModule::initialize()
{
  //Register collections
  //StoreArray<MCParticle> MCParticles;
  StoreArray<MCParticle>::registerPersistent();

  //Check parameters
  if (!FileSystem::fileExists(m_filename)) {
    B2ERROR("Parameter <Filename>: Could not open the file. The filename " << m_filename << " does not exist !")
  } else m_reader.open(m_filename);

  //Initialize the SAD reader.
  //Set the transformation from local SAD plane to global geant4 space.
  m_PipePartMatrix = new TGeoHMatrix("SADTrafo");
  m_PipePartMatrix->RotateZ(m_rotateParticles);

  GearDir ler("/Detector/SuperKEKB/LER/");
  GearDir her("/Detector/SuperKEKB/HER/");

  switch (m_accRing) {
    case 0 : m_PipePartMatrix->RotateY(ler.getDouble("angle") / Unit::deg);
      m_reader.initialize(m_PipePartMatrix, m_range, ReaderSAD::c_LER, m_readoutTime);
      break;
    case 1 : m_PipePartMatrix->RotateY(her.getDouble("angle") / Unit::deg);
      m_reader.initialize(m_PipePartMatrix, m_range, ReaderSAD::c_HER,  m_readoutTime);
      break;
    default: B2FATAL("Please specify a valid number for the accRing parameter (0 = LER, 1 = HER) !")
      break;
  }

  m_reader.setMomentumRes(m_pxRes, m_pyRes);
}


void SADInputModule::event()
{
  try {
    MCParticleGraph mpg;
    StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

    try {
      //----------------------------------
      //       Read the SAD data
      //----------------------------------
      switch (m_readMode) {
        case 0:  readSADParticle(m_reader, mpg);
          break;
        case 1:  readRealParticle(m_reader, mpg);
          break;
        case 2:  m_reader.addAllSADParticles(mpg);
          break;
        default: B2FATAL("Please specify a valid number for the readMode parameter !")
          break;
      }

      //----------------------------------
      // Generate MCParticles collection
      //----------------------------------
      mpg.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

    } catch (ReaderSAD::SADEndOfFile& exc) {
      B2DEBUG(10, exc.what())
      eventMetaDataPtr->setEndOfData();
      return;
    }
  } catch (runtime_error& exc) {
    B2ERROR(exc.what());
  }
}


//====================================================================
//                       Private methods
//====================================================================

void SADInputModule::readSADParticle(ReaderSAD& reader, MCParticleGraph& mpg)
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  double rate = reader.getSADParticle(mpg);
  if (rate < 0) return;
  eventMetaDataPtr->setGeneratedWeight(rate);
}

void SADInputModule::readRealParticle(ReaderSAD& reader, MCParticleGraph& mpg)
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  if (!reader.getRealParticle(mpg)) return;
  eventMetaDataPtr->setGeneratedWeight(1.0);
}
