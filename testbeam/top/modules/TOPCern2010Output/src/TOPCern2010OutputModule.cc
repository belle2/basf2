/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPCern2010Output/TOPCern2010OutputModule.h>
#include <top/geometry/TOPGeometryPar.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Hit classes
#include <top/dataobjects/TOPDigit.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCern2010Output)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCern2010OutputModule::TOPCern2010OutputModule() : Module()
  {
    // set module description
    setDescription("Output of CERN2010 test beam simulation to root ntuple (v20110331)");

    // Add parameters
    addParam("outputFileName", m_outputFileName, "Output file name",
             string("cern2010simu.root"));

    // initialize other private data members
    m_file = NULL;
    m_tana = NULL;
    for (int i = 0; i < c_NumChannels; i++) m_adc[i] = 0;
    for (int i = 0; i < c_NumChannels; i++) m_tdc[i] = 0;
    for (int i = 0; i < 4; i++) m_data_tc[i] = 0;
    for (int i = 0; i < 4; i++) m_data_trig[i] = 0;
    for (int i = 0; i < 4; i++) m_data_veto[i] = 0;
    m_nhit = 0;
    m_x1 = 0;
    m_y1 = 0;
    m_x2 = 0;
    m_y2 = 0;
    m_x3 = 0;
    m_y3 = 0;
    m_x4 = 0;
    m_y4 = 0;
    m_theta = 0;
    m_phi = 0;
    m_x = 0;
    m_y = 0;

  }

  TOPCern2010OutputModule::~TOPCern2010OutputModule()
  {
  }

  void TOPCern2010OutputModule::initialize()
  {

    m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
    m_tana = new TTree("ana", "ana");

    m_tana->Branch("adc", m_adc, "adc[36]/D");
    m_tana->Branch("tdc", m_tdc, "tdc[36]/D");
    m_tana->Branch("adc_tc1", &m_data_tc[0], "adc_tc1/D");
    m_tana->Branch("adc_tc2", &m_data_tc[1], "adc_tc2/D");
    m_tana->Branch("tdc_tc1", &m_data_tc[2], "tdc_tc1/D");
    m_tana->Branch("tdc_tc2", &m_data_tc[3], "tdc_tc2/D");
    m_tana->Branch("adc_trig1", &m_data_trig[0], "adc_trig1/D");
    m_tana->Branch("adc_trig2", &m_data_trig[1], "adc_trig2/D");
    m_tana->Branch("tdc_trig1", &m_data_trig[2], "tdc_trig1/D");
    m_tana->Branch("tdc_trig2", &m_data_trig[3], "tdc_trig2/D");
    m_tana->Branch("adc_veto1", &m_data_veto[0], "adc_veto1/D");
    m_tana->Branch("adc_veto2", &m_data_veto[1], "adc_veto2/D");
    m_tana->Branch("tdc_veto1", &m_data_veto[2], "tdc_veto1/D");
    m_tana->Branch("tdc_veto2", &m_data_veto[3], "tdc_veto2/D");
    m_tana->Branch("nhit", &m_nhit, "nhit/I");
    m_tana->Branch("x1", &m_x1, "x1/D");
    m_tana->Branch("y1", &m_y1, "y1/D");
    m_tana->Branch("x2", &m_x2, "x2/D");
    m_tana->Branch("y2", &m_y2, "y2/D");
    m_tana->Branch("x3", &m_x3, "x3/D");
    m_tana->Branch("y3", &m_y3, "y3/D");
    m_tana->Branch("x4", &m_x4, "x4/D");
    m_tana->Branch("y4", &m_y4, "y4/D");
    m_tana->Branch("theta", &m_theta, "theta/D");
    m_tana->Branch("phi", &m_phi, "phi/D");
    m_tana->Branch("x", &m_x, "x/D");
    m_tana->Branch("y", &m_y, "y/D");

  }

  void TOPCern2010OutputModule::beginRun()
  {
  }

  void TOPCern2010OutputModule::event()
  {

    StoreArray<TOPDigit> topDigits;

    for (int ich = 0; ich < c_NumChannels; ich++) m_tdc[ich] = 4096;

    int ndigi = topDigits.getEntries();
    for (int i = 0; i < ndigi; i++) {
      TOPDigit* digi = topDigits[i];
      int ich = c_NumChannels - getOldNumbering(digi->getPixelID());
      if (digi->getTDC() < m_tdc[ich]) m_tdc[ich] = digi->getTDC(); // single hit TDC
    }
    m_nhit = 0;
    for (int ich = 0; ich < c_NumChannels; ich++) {
      if (m_tdc[ich] < 4096) m_nhit++;
    }
    m_tana->Fill();

  }


  void TOPCern2010OutputModule::endRun()
  {
  }

  void TOPCern2010OutputModule::terminate()
  {
    m_file->cd();
    m_tana->Write();
    m_file->Close();
  }

  int TOPCern2010OutputModule::getOldNumbering(int pixelID) const
  {
    if (pixelID == 0) return 0;

    const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
    const auto& pmtArray = geo->getModule(1).getPMTArray();
    int Npmtx = pmtArray.getNumColumns();
    int Npadx = pmtArray.getPMT().getNumColumns();
    int Npady = pmtArray.getPMT().getNumRows();

    pixelID--;
    int nx = Npmtx * Npadx;
    int i = pixelID % nx;
    int j = pixelID / nx;
    int ix = i % Npadx;
    int ipmtx = i / Npadx;
    int iy = j % Npady;
    int ipmty = j / Npady;

    ix = Npadx - 1 - ix;
    ipmtx = Npmtx - 1 - ipmtx;

    return ix + Npadx * (iy + Npady * (ipmtx + Npmtx * ipmty)) + 1;

  }


} // end Belle2 namespace

