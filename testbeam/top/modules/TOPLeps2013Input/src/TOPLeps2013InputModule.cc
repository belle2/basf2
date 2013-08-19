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
#include <testbeam/top/modules/TOPLeps2013Input/TOPLeps2013InputModule.h>

#include <framework/core/ModuleManager.h>

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
#include <top/dataobjects/TOPDigit.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPLeps2013Input)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPLeps2013InputModule::TOPLeps2013InputModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Input of LEPS 2013 test beam data from a specific root ntuple (top tree)");
    setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    // Add parameters
    addParam("inputFileName", m_inputFileName, "Input file name",
             string(""));

    // initialize other private data members
    m_file = NULL;
    m_treeTop = NULL;
    m_top.clear();
    m_topgp = NULL;

    m_numPMTchannels = 0;
    m_numChannels = 0;
    m_tdcWidth = 0;
    m_tdcOverflow = 0;
    m_numEntries = 0;
    m_entryCounter = 0;

  }

  TOPLeps2013InputModule::~TOPLeps2013InputModule()
  {
  }

  void TOPLeps2013InputModule::initialize()
  {
    if (m_inputFileName.empty()) B2FATAL("file name not given");
    char filename[m_inputFileName.size() + 1];
    strcpy(filename, m_inputFileName.c_str());
    m_file = new TFile(filename);
    if (!m_file->IsOpen()) B2FATAL("can't open file " << filename);

    m_treeTop = (TTree*)m_file->Get("top");

    m_treeTop->SetBranchAddress("runNum", &(m_top.runNum));
    m_treeTop->SetBranchAddress("eventNum", &(m_top.eventNum));
    m_treeTop->SetBranchAddress("eventflag", &(m_top.eventflag));
    m_treeTop->SetBranchAddress("eventtag_top", &(m_top.eventtag_top));

    m_treeTop->SetBranchAddress("nhit", &(m_top.nhit));
    m_treeTop->SetBranchAddress("pmt", &(m_top.pmtid_mcp));
    m_treeTop->SetBranchAddress("ch", &(m_top.ch_mcp));
    m_treeTop->SetBranchAddress("tdc0", &(m_top.tdc0_mcp));
    m_treeTop->SetBranchAddress("adc0", &(m_top.adc0_mcp));
    m_treeTop->SetBranchAddress("pmtflag", &(m_top.pmtflag_mcp));

    m_treeTop->SetBranchAddress("rf", &(m_top.rf));
    m_treeTop->SetBranchAddress("rf_time", &(m_top.rf_time));
    m_treeTop->SetBranchAddress("bunchNum", &(m_top.bunchNum));

    m_treeTop->SetBranchAddress("trigS_tdc", &(m_top.trigS_tdc));
    m_treeTop->SetBranchAddress("trigS_adc", &(m_top.trigS_adc));
    m_treeTop->SetBranchAddress("trigM_tdc", &(m_top.trigM_tdc));
    m_treeTop->SetBranchAddress("trigM_adc", &(m_top.trigM_adc));
    m_treeTop->SetBranchAddress("timing_tdc", &(m_top.timing_tdc));
    m_treeTop->SetBranchAddress("timing_adc", &(m_top.timing_adc));
    m_treeTop->SetBranchAddress("veto_adc", &(m_top.veto_adc));
    m_treeTop->SetBranchAddress("ratemon", &(m_top.ratemon));

    m_treeTop->SetBranchAddress("tdc0_ch", &(m_top.tdc0_ch));
    m_treeTop->SetBranchAddress("adc0_ch", &(m_top.adc0_ch));
    m_treeTop->SetBranchAddress("pmtflag_ch", &(m_top.pmtflag_ch));
    m_treeTop->SetBranchAddress("tdc", &(m_top.tdc_mcp));
    m_treeTop->SetBranchAddress("tdc_ch", &(m_top.tdc_ch));

    m_treeTop->SetBranchAddress("trk_x", &(m_top.trk_x));
    m_treeTop->SetBranchAddress("trk_z_x", &(m_top.trk_z_x));
    m_treeTop->SetBranchAddress("trk_qual_x", &(m_top.trk_qual_x));

    m_treeTop->SetBranchAddress("trk_y", &(m_top.trk_y));
    m_treeTop->SetBranchAddress("trk_z_y", &(m_top.trk_z_y));
    m_treeTop->SetBranchAddress("trk_qual_y", &(m_top.trk_qual_y));

    m_treeTop->SetBranchAddress("trk1_x", &(m_top.trk1_x));
    m_treeTop->SetBranchAddress("trk1_y", &(m_top.trk1_y));
    m_treeTop->SetBranchAddress("trk1_z", &(m_top.trk1_z));

    m_treeTop->SetBranchAddress("trk2_x", &(m_top.trk2_x));
    m_treeTop->SetBranchAddress("trk2_y", &(m_top.trk2_y));
    m_treeTop->SetBranchAddress("trk2_z", &(m_top.trk2_z));

    m_treeTop->SetBranchAddress("trk_top_x", &(m_top.trk_top_x));
    m_treeTop->SetBranchAddress("trk_top_y", &(m_top.trk_top_y));
    m_treeTop->SetBranchAddress("trk_top_z", &(m_top.trk_top_z));

    m_numEntries = m_treeTop->GetEntries();
    m_entryCounter = 0;

    // data store objects registration
    StoreObjPtr<EventMetaData>::registerPersistent();
    StoreArray<TOPDigit>::registerPersistent();

  }

  void TOPLeps2013InputModule::beginRun()
  {
  }

  void TOPLeps2013InputModule::event()
  {
    // initialize
    if (!m_topgp) {
      m_topgp = TOP::TOPGeometryPar::Instance();
      if (m_topgp->getNbars() == 0) B2FATAL("Geometry not loaded");

      m_topgp->setBasfUnits();
      m_numPMTchannels = m_topgp->getNpadx() * m_topgp->getNpady();
      m_numChannels = m_topgp->getNpmtx() * m_topgp->getNpmty() * m_numPMTchannels;
      if (m_numChannels > 512) B2FATAL("Number of channels > 512");
      m_tdcWidth = m_topgp->getTDCbitwidth() * Unit::ns / Unit::ps;
      m_tdcOverflow = 1 << m_topgp->getTDCbits();
    }
    m_top.clear();
    m_topgp->setBasfUnits();

    // create data store objects
    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.create();
    StoreArray<TOPDigit> digits;
    digits.create();

    // check entry counter
    if (m_entryCounter == m_numEntries) {
      evtMetaData->setEndOfData(); // stop event processing
      return;
    }

    // read event (TODO: only for m_top.eventflag == 1)
    m_treeTop->GetEntry(m_entryCounter);
    m_entryCounter++;

    // set event metadata
    evtMetaData->setEvent(m_top.eventNum);
    evtMetaData->setRun(m_top.runNum);

    // write good data to data store
    for (int i = 0; i < (int)m_top.nhit; i++) {
      if (m_top.pmtflag_mcp[i] != 1) continue;
      int ich = (m_top.pmtid_mcp[i] - 1) * m_numPMTchannels + m_top.ch_mcp[i];
      int TDC = m_top.tdc0_mcp[i] / m_tdcWidth;
      new(digits.nextFreeAddress()) TOPDigit(1, ich, TDC);
    }

  }


  void TOPLeps2013InputModule::endRun()
  {
  }

  void TOPLeps2013InputModule::terminate()
  {
  }

  void TOPLeps2013InputModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

