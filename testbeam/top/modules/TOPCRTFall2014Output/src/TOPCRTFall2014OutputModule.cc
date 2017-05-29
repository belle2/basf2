/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Matthew Barrett                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPCRTFall2014Output/TOPCRTFall2014OutputModule.h>
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

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPBarHit.h>
#include <testbeam/top/dataobjects/TOPTBDigit.h>

#include <TRandom3.h>
#include <string>
#include <vector>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCRTFall2014Output)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCRTFall2014OutputModule::TOPCRTFall2014OutputModule() : Module()
  {
    // set module description
    setDescription("Output of Cosmic ray test Autumn/Fall 2014 simulation to a specific root TTree format");

    // Add parameters
    addParam("outputFileName", m_outputFileName, "Output file name",
             string("CRTFall2014simulation.root"));
    addParam("randomize", m_randomize, "Randomize time within TDC bin",
             true);
    addParam("t0", m_t0, "common offset to be added to photon TDC times [ns]", 0.0);
    addParam("multipleHits", m_multipleHits, "Allow multiple hits in a single channel within an event", false);

    // initialize other private data members
    m_file = NULL;
    m_treeTop = NULL;
    m_top.clear();

    m_numPMTchannels = 0;
    m_numChannels = 0;
    m_tdcWidth = 0;
    m_tdcOverflow = 0;

  }

  TOPCRTFall2014OutputModule::~TOPCRTFall2014OutputModule()
  {
  }

  void TOPCRTFall2014OutputModule::initialize()
  {
    m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
    m_treeTop = new TTree("top", "top data tree (simulation)");

    m_treeTop->Branch("runNum", &(m_top.runNum), "runNum/I");
    m_treeTop->Branch("eventNum", &(m_top.eventNum), "eventNum/I");
    m_treeTop->Branch("eventflag", &(m_top.eventflag), "eventflag/S");

    m_treeTop->Branch("nhit", &(m_top.nhit), "nhit/S");
    m_treeTop->Branch("pmt", &(m_top.pmtid_mcp), "pmt[nhit]/S");
    m_treeTop->Branch("ch", &(m_top.ch_mcp), "ch[nhit]/S");
    m_treeTop->Branch("tdc0", &(m_top.tdc0_mcp), "tdc0[nhit]/F");
    m_treeTop->Branch("adc0", &(m_top.adc0_mcp), "adc0[nhit]/F");
    m_treeTop->Branch("pmtflag", &(m_top.pmtflag_mcp), "pmtflag[nhit]/S");

    m_treeTop->Branch("trig12", &(m_top.trig12), "trig12/F");
    m_treeTop->Branch("trig13", &(m_top.trig13), "trig13/F");
    m_treeTop->Branch("trig14", &(m_top.trig14), "trig14/F");

    m_treeTop->Branch("nDT1", &(m_top.nDT1), "nDT1/I");
    m_treeTop->Branch("nDT2", &(m_top.nDT2), "nDT2/I");
    m_treeTop->Branch("nDT3", &(m_top.nDT3), "nDT3/I");
    m_treeTop->Branch("nDT4", &(m_top.nDT4), "nDT4/I");

    m_treeTop->Branch("DT1ch", &(m_top.DT1ch), "DT1ch[nDT1]/I");
    m_treeTop->Branch("DT2ch", &(m_top.DT2ch), "DT2ch[nDT2]/I");
    m_treeTop->Branch("DT3ch", &(m_top.DT3ch), "DT3ch[nDT3]/I");
    m_treeTop->Branch("DT4ch", &(m_top.DT4ch), "DT4ch[nDT4]/I");
    m_treeTop->Branch("DT1tdc", &(m_top.DT1ch), "DT1tdc[nDT1]/I");
    m_treeTop->Branch("DT2tdc", &(m_top.DT2ch), "DT2tdc[nDT2]/I");
    m_treeTop->Branch("DT3tdc", &(m_top.DT3ch), "DT3tdc[nDT3]/I");
    m_treeTop->Branch("DT4tdc", &(m_top.DT4ch), "DT4tdc[nDT4]/I");

    m_treeTop->Branch("TOF1tdc", &(m_top.TOF1tdc), "TOF1tdc[4]/I");
    m_treeTop->Branch("TOF1adc", &(m_top.TOF1adc), "TOF1adc[4]/I");
    m_treeTop->Branch("TOF2tdc", &(m_top.TOF2tdc), "TOF2tdc[4]/I");
    m_treeTop->Branch("TOF2adc", &(m_top.TOF2adc), "TOF2adc[4]/I");
    m_treeTop->Branch("TOF3tdc", &(m_top.TOF3tdc), "TOF3tdc[4]/I");
    m_treeTop->Branch("TOF3adc", &(m_top.TOF3adc), "TOF3adc[4]/I");
    m_treeTop->Branch("TOF4tdc", &(m_top.TOF4tdc), "TOF4tdc[4]/I");
    m_treeTop->Branch("TOF4adc", &(m_top.TOF4adc), "TOF4adc[4]/I");

    const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
    const auto& module = geo->getModule(1);
    m_numPMTchannels = module.getPMTArray().getPMT().getNumPixels();
    m_numChannels = module.getPMTArray().getNumPixels();
    if (m_numChannels > 512) B2FATAL("Number of channels > 512");
    m_tdcWidth = geo->getNominalTDC().getBinWidth() * Unit::ns / Unit::ps;
    m_tdcOverflow = geo->getNominalTDC().getOverflowValue();
  }

  void TOPCRTFall2014OutputModule::beginRun()
  {
  }

  void TOPCRTFall2014OutputModule::event()
  {

    m_top.clear();

    StoreObjPtr<EventMetaData> evtMetaData;
    m_top.runNum = evtMetaData->getRun();
    m_top.eventNum = evtMetaData->getEvent();
    m_top.eventflag = 1;

    std::vector<float> TDC;
    TDC.reserve(m_numChannels);

    for (int i = 0; i < m_numChannels; ++i) {TDC.at(i) = m_tdcOverflow;}

    StoreArray<TOPDigit> topDigits;
    int nEntries(topDigits.getEntries());

    for (int i = 0; i < nEntries; ++i) {
      TOPDigit* digi = topDigits[i];

      int ich = getOldNumbering(digi->getPixelID()) - 1;

      float tdc = digi->getRawTime();
      if (m_randomize) {tdc += gRandom->Rndm();}

      if (m_multipleHits) {
        m_top.pmtid_mcp[i] = ich / m_numPMTchannels + 1;
        m_top.ch_mcp[i] = ich % m_numPMTchannels + 1;
        m_top.tdc0_mcp[i] = tdc * m_tdcWidth + m_t0 * Unit::ns / Unit::ps;
        m_top.tdc_mcp[i] = m_top.tdc0_mcp[i];
        m_top.pmtflag_mcp[i] = 1;
      }

    }

    for (int ich = 0; ich < m_numChannels; ich++) {
      if (TDC[ich] == m_tdcOverflow) {continue;}
      int i = m_top.nhit;
      m_top.nhit++;
      float tdc = TDC[ich] * m_tdcWidth + m_t0 * Unit::ns / Unit::ps;
      if (!m_multipleHits) {
        m_top.pmtid_mcp[i] = ich / m_numPMTchannels + 1;
        m_top.ch_mcp[i] = ich % m_numPMTchannels + 1;
        m_top.tdc0_mcp[i] = tdc;
        m_top.tdc_mcp[i] = tdc;
        m_top.pmtflag_mcp[i] = 1;
      }

    }

    if (m_multipleHits) {m_top.nhit = nEntries;}
    m_treeTop->Fill();

  }


  void TOPCRTFall2014OutputModule::endRun()
  {
  }

  void TOPCRTFall2014OutputModule::terminate()
  {
    m_file->cd();
    m_treeTop->Write();
    m_file->Close();
  }

  int TOPCRTFall2014OutputModule::getOldNumbering(int pixelID) const
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

