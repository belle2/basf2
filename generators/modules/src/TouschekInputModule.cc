/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/TouschekInputModule.h>

#include <framework/core/ModuleUtils.h>

#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoNode.h>

#include <fstream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TouschekInputModule, "TouschekInput")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TouschekInputModule::TouschekInputModule() : Module()
{
  //Set module properties
  setDescription("Reads the Touschek data from a TURTLE file and stores it into the MCParticle collection.");
  setPropertyFlags(c_TriggersEndOfData | c_ReadsDataSingleProcess | c_RequiresSingleProcess);

  //Parameter definition
  addParam("FilenameHER",  m_filenameHER, string("HER.dat"), "The filename of the HER TURTLE input file.", true);
  addParam("FilenameLER",  m_filenameLER, string("LER.dat"), "The filename of the LER TURTLE input file.", true);
  addParam("ReadHER", m_readHER, true, "Set to false to skip reading the HER data.");
  addParam("ReadLER", m_readLER, true, "Set to false to skip reading the LER data.");
  addParam("MaxParticles", m_maxParticles, -1, "The maximum number of particles per event that should be read. -1 means all of the particles are read.");

  //Create and initialize member variables
  m_herPipePartMatrix = new TGeoHMatrix("TouschekPlaneHER");
  m_lerPipePartMatrix = new TGeoHMatrix("TouschekPlaneLER");
  m_readerHER = new TouschekReader(m_herPipePartMatrix, 11);  //HER: electrons
  m_readerLER = new TouschekReader(m_herPipePartMatrix, -11); //LER: positrons
}


TouschekInputModule::~TouschekInputModule()
{
  delete m_readerHER;
  delete m_readerLER;
}


void TouschekInputModule::initialize()
{
  //Check parameters
  if ((m_readHER) && (!ModuleUtils::fileNameExists(m_filenameHER))) {
    B2ERROR("Parameter <FilenameHER>: Could not open the file. The filename " << m_filenameHER << " does not exist !")
  } else m_readerHER->open(m_filenameHER);

  if ((m_readLER) && (!ModuleUtils::fileNameExists(m_filenameLER))) {
    B2ERROR("Parameter <FilenameLER>: Could not open the file. The filename " << m_filenameLER << " does not exist !")
  } else m_readerLER->open(m_filenameLER);

  //Get the transformation from local Touschek plane space to global geant4 space
  //For the HER
  gGeoManager->CdTop();
  gGeoManager->cd("/IR_1/HERUpstreamFlange1_1Tube_1");
  double herTouschekPlaneTrans[3] = {0.0, 0.0, 0.0};
  herTouschekPlaneTrans[2] = gGeoManager->GetCurrentMatrix()->GetTranslation()[2] + 50.0; //TURTLE data is at -50 cm
  m_herPipePartMatrix->SetTranslation(herTouschekPlaneTrans);
  m_herPipePartMatrix->MultiplyLeft(gGeoManager->GetCurrentMatrix());

  //For the LER
  gGeoManager->CdTop();
  gGeoManager->cd("/IR_1/LERUpstreamFlange1_1Tube_1");
  double lerTouschekPlaneTrans[3] = {0.0, 0.0, 0.0};
  lerTouschekPlaneTrans[2] = -gGeoManager->GetCurrentMatrix()->GetTranslation()[2] + 50.0; //TURTLE data is at 50 cm
  m_lerPipePartMatrix->SetTranslation(lerTouschekPlaneTrans);
  m_lerPipePartMatrix->MultiplyLeft(gGeoManager->GetCurrentMatrix());
}


void TouschekInputModule::event()
{
  int readHERParticles = 0;
  int readLERParticles = 0;

  try {
    //Read the data
    MCParticleGraph mpg;
    if (m_readHER) readHERParticles = m_readerHER->getParticles(m_maxParticles, mpg); //HER: electrons
    if (m_readLER) readLERParticles = m_readerLER->getParticles(m_maxParticles, mpg); //LER: positrons

    if ((readHERParticles <= 0) && (readLERParticles <= 0)) {
      setProcessRecordType(prt_EndOfData);
    } else {
      //Generate MCParticle list
      mpg.generateList(DEFAULT_MCPARTICLES, MCParticleGraph::set_decay_info | MCParticleGraph::check_cyclic);

      B2INFO("Read " << readHERParticles << " e- particles (HER).")
      B2INFO("Read " << readLERParticles << " e+ particles (LER).")
    }
  } catch (runtime_error &exc) {
    B2ERROR(exc.what());
  }
}
