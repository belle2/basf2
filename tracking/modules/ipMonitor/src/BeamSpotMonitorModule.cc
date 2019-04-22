/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Gaetano De Marino                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/ipMonitor/BeamSpotMonitorModule.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BeamSpotMonitor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BeamSpotMonitorModule::BeamSpotMonitorModule() : Module()
{
  // Set module properties
  setDescription("Module for the monitoring of the BeamSpot position and size");

  // Parameter definitions
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("BeamSpotMonitor.root"));
}

void BeamSpotMonitorModule::initialize()
{

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //tree initialization
  m_tree = new TTree("bspt", "RECREATE");
  b_exp = m_tree->Branch("exp", &m_exp, "exp/i");
  b_run = m_tree->Branch("run", &m_run, "run/i");
  b_x = m_tree->Branch("x", &m_x, "x/d");
  b_y = m_tree->Branch("y", &m_y, "y/d");
  b_z = m_tree->Branch("z", &m_z, "z/d");
  b_xErr = m_tree->Branch("xErr", &m_xErr, "xErr/d");
  b_yErr = m_tree->Branch("yErr", &m_yErr, "yErr/d");
  b_zErr = m_tree->Branch("zErr", &m_zErr, "zErr/d");
  b_xSize = m_tree->Branch("xSize", &m_xSize, "xSize/d");
  b_ySize = m_tree->Branch("ySize", &m_ySize, "ySize/d");
  b_zSize = m_tree->Branch("zSize", &m_zSize, "zSize/d");

}

void BeamSpotMonitorModule::beginRun()
{

  //  m_BeamSpotDB = unique_ptr<Belle2::DBObjPtr<Belle2::BeamSpot>>(new Belle2::DBObjPtr<Belle2::BeamSpot>());
  //  m_BeamSpotDB = Belle2::DBObjPtr<Belle2::BeamSpot>;

  if (! m_BeamSpotDB.isValid())
    B2WARNING("No valid BeamSpot for the requested IoV");
  else
    m_BeamSpot = *m_BeamSpotDB;

}

void BeamSpotMonitorModule::event()
{

  StoreObjPtr<EventMetaData> meta;
  m_exp = meta->getExperiment();
  m_run = meta->getRun();
  B2INFO("monitoring beam spot for experiment = " << m_exp << ", run = " << m_run);

  if (! m_BeamSpotDB.isValid())
    return;

  //retrieve vertex position
  m_x = m_BeamSpot.getVertex().X();
  m_y = m_BeamSpot.getVertex().Y();
  m_z = m_BeamSpot.getVertex().Z();

  //retrieve vertex position error
  TMatrixDSym posErr = m_BeamSpot.getPositionError();
  m_xErr = sqrt(posErr[0][0]);
  m_yErr = sqrt(posErr[1][1]);
  m_zErr = sqrt(posErr[2][2]);

  //retrieve beam spot size
  TMatrixDSym size = m_BeamSpot.getSize();
  m_xSize = sqrt(size[0][0]);
  m_ySize = sqrt(size[1][1]);
  m_zSize = sqrt(size[2][2]);

  m_tree->Fill();

}

void BeamSpotMonitorModule::terminate()
{
  //  B2RESULT("******************************************");
  //  B2RESULT("** UNIQUE IDs of calibration DB objects **");
  //  B2RESULT("");
  //  if (m_BeamSpotDB.isValid())
  //    B2RESULT("   - BeamSpot:" << m_BeamSpotDB.getUniqueID());
  //  else
  //    B2WARNING("No valid BeamSpot for the requested IoV");

  if (m_rootFilePtr != NULL) {

    m_rootFilePtr->cd();

    //write the tree
    m_tree->Write();

    m_rootFilePtr->Close();
    //    B2RESULT("The rootfile containing the informations on the BeamSpot has been filled and closed.");

  }
}

