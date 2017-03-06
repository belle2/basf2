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
#include <testbeam/top/modules/TOPLeps2013Output/TOPLeps2013OutputModule.h>
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

#include <TRandom3.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPLeps2013Output)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPLeps2013OutputModule::TOPLeps2013OutputModule() : Module()
  {
    // set module description
    setDescription("Output of LEPS 2013 test beam simulation to a specific root ntuple (top tree)");

    // Add parameters
    addParam("outputFileName", m_outputFileName, "Output file name",
             string("leps2013simu.root"));
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

  TOPLeps2013OutputModule::~TOPLeps2013OutputModule()
  {
  }

  void TOPLeps2013OutputModule::initialize()
  {

    m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
    m_treeTop = new TTree("top", "top data tree (simulation)");

    m_treeTop->Branch("runNum", &(m_top.runNum), "runNum/I");
    m_treeTop->Branch("eventNum", &(m_top.eventNum), "eventNum/I");
    m_treeTop->Branch("eventflag", &(m_top.eventflag), "eventflag/S");
    m_treeTop->Branch("eventtag_top", &(m_top.eventtag_top), "eventtag_top/S");

    m_treeTop->Branch("nhit", &(m_top.nhit), "nhit/S");
    m_treeTop->Branch("pmt", &(m_top.pmtid_mcp), "pmt[nhit]/S");
    m_treeTop->Branch("ch", &(m_top.ch_mcp), "ch[nhit]/S");
    m_treeTop->Branch("tdc0", &(m_top.tdc0_mcp), "tdc0[nhit]/F");
    m_treeTop->Branch("adc0", &(m_top.adc0_mcp), "adc0[nhit]/F");
    m_treeTop->Branch("pmtflag", &(m_top.pmtflag_mcp), "pmtflag[nhit]/S");

    m_treeTop->Branch("rf", &(m_top.rf), "rf[4]/I");
    m_treeTop->Branch("rf_time", &(m_top.rf_time), "rf_time/F");
    m_treeTop->Branch("bunchNum", &(m_top.bunchNum), "bunchNum/S");

    m_treeTop->Branch("trigS_tdc", &(m_top.trigS_tdc), "trigS_tdc[2]/I");
    m_treeTop->Branch("trigS_adc", &(m_top.trigS_adc), "trigS_adc[2]/I");
    m_treeTop->Branch("trigM_tdc", &(m_top.trigM_tdc), "trigM_tdc[2]/I");
    m_treeTop->Branch("trigM_adc", &(m_top.trigM_adc), "trigM_adc[2]/I");
    m_treeTop->Branch("timing_tdc", &(m_top.timing_tdc), "timing_tdc/I");
    m_treeTop->Branch("timing_adc", &(m_top.timing_adc), "timing_adc/I");
    m_treeTop->Branch("veto_adc", &(m_top.veto_adc), "veto_adc[2]/I");
    m_treeTop->Branch("ratemon", &(m_top.ratemon), "ratemon/I");

    m_treeTop->Branch("tdc0_ch", &(m_top.tdc0_ch), "tdc0_ch[512]/F");
    m_treeTop->Branch("adc0_ch", &(m_top.adc0_ch), "adc0_ch[512]/F");
    m_treeTop->Branch("pmtflag_ch", &(m_top.pmtflag_ch), "pmtflag_ch[512]/I");
    m_treeTop->Branch("tdc", &(m_top.tdc_mcp), "tdc[nhit]/F");
    m_treeTop->Branch("tdc_ch", &(m_top.tdc_ch), "tdc_ch[512]/F");

    m_treeTop->Branch("trk_x", &(m_top.trk_x), "trk_x[2]/F");
    m_treeTop->Branch("trk_z_x", &(m_top.trk_z_x), "trk_z_x[2]/F");
    m_treeTop->Branch("trk_qual_x", &(m_top.trk_qual_x), "trk_qual_x[2]/I");

    m_treeTop->Branch("trk_y", &(m_top.trk_y), "trk_y[2]/F");
    m_treeTop->Branch("trk_z_y", &(m_top.trk_z_y), "trk_z_y[2]/F");
    m_treeTop->Branch("trk_qual_y", &(m_top.trk_qual_y), "trk_qual_y[2]/I");

    m_treeTop->Branch("trk1_x", &(m_top.trk1_x), "trk1_x/F");
    m_treeTop->Branch("trk1_y", &(m_top.trk1_y), "trk1_y/F");
    m_treeTop->Branch("trk1_z", &(m_top.trk1_z), "trk1_z/F");

    m_treeTop->Branch("trk2_x", &(m_top.trk2_x), "trk2_x/F");
    m_treeTop->Branch("trk2_y", &(m_top.trk2_y), "trk2_y/F");
    m_treeTop->Branch("trk2_z", &(m_top.trk2_z), "trk2_z/F");

    m_treeTop->Branch("trk_top_x", &(m_top.trk_top_x), "trk_top_x/F");
    m_treeTop->Branch("trk_top_y", &(m_top.trk_top_y), "trk_top_y/F");
    m_treeTop->Branch("trk_top_z", &(m_top.trk_top_z), "trk_top_z/F");

    const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
    const auto& pmtArray = geo->getModule(1).getPMTArray();
    m_numPMTchannels = pmtArray.getPMT().getNumPixels();
    m_numChannels = pmtArray.getNumPixels();
    if (m_numChannels > 512) B2FATAL("Number of channels > 512");
    m_tdcWidth = geo->getNominalTDC().getBinWidth() * Unit::ns / Unit::ps;
    m_tdcOverflow = geo->getNominalTDC().getOverflowValue();

  }

  void TOPLeps2013OutputModule::beginRun()
  {
  }

  void TOPLeps2013OutputModule::event()
  {
    m_top.clear();

    StoreObjPtr<EventMetaData> evtMetaData;
    m_top.runNum = evtMetaData->getRun();
    m_top.eventNum = evtMetaData->getEvent();
    m_top.eventflag = 1;

    float TDC[512]; // m_numChannels cannot exceed 512, see initialize()
    for (int i = 0; i < m_numChannels; i++) TDC[i] = m_tdcOverflow;

    StoreArray<TOPDigit> topDigits;
    int nEntries(topDigits.getEntries());
    for (int i = 0; i < nEntries; ++i) {
      TOPDigit* digi = topDigits[i];
      int ich = getOldNumbering(digi->getPixelID()) - 1;
      float tdc(digi->getTDC());
      if (m_randomize) {tdc += gRandom->Rndm();}

      if (m_multipleHits) {
        m_top.pmtid_mcp[i] = ich / m_numPMTchannels + 1;
        m_top.ch_mcp[i] = ich % m_numPMTchannels + 1;
        m_top.tdc0_mcp[i] = tdc * m_tdcWidth + m_t0 * Unit::ns / Unit::ps;
        m_top.tdc_mcp[i] = m_top.tdc0_mcp[i];
        m_top.pmtflag_mcp[i] = 1;
      }
      if (int(tdc) < int(TDC[ich])) {TDC[ich] = tdc;} // for single hit TDC
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
      m_top.tdc0_ch[ich] = tdc;
      m_top.pmtflag_ch[ich] = m_top.pmtflag_mcp[i];
      m_top.tdc_ch[ich] = tdc;
    }

    if (m_multipleHits) {m_top.nhit = nEntries;}
    m_treeTop->Fill();

  }


  void TOPLeps2013OutputModule::endRun()
  {
  }

  void TOPLeps2013OutputModule::terminate()
  {
    m_file->cd();
    m_treeTop->Write();
    m_file->Close();
  }

  int TOPLeps2013OutputModule::getOldNumbering(int pixelID) const
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

