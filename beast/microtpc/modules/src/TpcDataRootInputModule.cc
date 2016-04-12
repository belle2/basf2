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
#include <beast/microtpc/modules/TpcDataRootInputModule.h>
#include <beast/microtpc/dataobjects/MicrotpcDataHit.h>
#include <beast/microtpc/modules/TpcTree.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework core
#include <framework/core/ModuleManager.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/io/RootIOUtilities.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace std;

namespace Belle2 {

  using namespace RootIOUtilities;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TpcDataRootInput)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TpcDataRootInputModule::TpcDataRootInputModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Input of Phase 1 BEAST micro-TPC data from a specific root ntuple (Data_TPC tree)");
    /*
    // Add parameters
    addParam("inputFileName", m_inputFileName, "Input file name",
             string(""));
    */

    // Add parameters
    addParam("inputFileNames", m_inputFileNames,
             "List of files with measured beam background ");

    // initialize other private data members
    m_file = NULL;
    m_tree = NULL;
    m_treeTpc = NULL;
    m_treeTpcPar = NULL;
    m_tpc.clear();
    //m_tpcgp = NULL;

    m_numEntries = 0;
    m_entryCounter = 0;

  }

  TpcDataRootInputModule::~TpcDataRootInputModule()
  {
  }

  void TpcDataRootInputModule::initialize()
  {
    /*
    if (m_inputFileName.empty()) B2FATAL("file name not given");

    m_file = new TFile(m_inputFileName.c_str());
    if (!m_file->IsOpen()) B2FATAL("can't open file " << m_inputFileName);

    m_tree = (TTree*)m_file->Get("Data_TPC");
    if (!m_tree) B2FATAL("No tpc tree found in file " << m_inputFileName);

    m_treePar = (TTree*)m_file->Get("ParData_TPC");
    if (!m_treePar) B2FATAL("No parameters tpc tree found in file " << m_inputFileName);
    */

    loadDictionaries();

    // expand possible wildcards
    m_inputFileNames = expandWordExpansions(m_inputFileNames);
    if (m_inputFileNames.empty()) {
      B2FATAL("No valid files specified!");
    }

    // check files
    TDirectory* dir = gDirectory;
    for (const string& fileName : m_inputFileNames) {
      TFile* f = TFile::Open(fileName.c_str(), "READ");
      if (!f or !f->IsOpen()) {
        B2FATAL("Couldn't open input file " + fileName);
      }
      delete f;
    }
    dir->cd();

    // get event TTree
    //m_tree = new TChain(c_treeNames[DataStore::c_Event].c_str());
    m_tree = new TChain("Data_TPC");
    for (const string& fileName : m_inputFileNames) {
      m_tree->AddFile(fileName.c_str());
    }
    m_numEvents = m_tree->GetEntries();
    cout << " m_numEvents " << m_numEvents << endl;
    if (m_numEvents == 0) B2ERROR(c_treeNames[DataStore::c_Event] << " has no entires");
    m_eventCount = 0;

    m_tree->SetBranchAddress("pxhits", &(m_tpc.m_pxhits));
    m_tree->SetBranchAddress("evtnb", &(m_tpc.m_evtnb));
    m_tree->SetBranchAddress("relative_BCID", (m_tpc.m_bcid));
    m_tree->SetBranchAddress("column", (m_tpc.m_column));
    m_tree->SetBranchAddress("row", (m_tpc.m_row));
    m_tree->SetBranchAddress("tot", (m_tpc.m_tot));
    m_tree->SetBranchAddress("timestamp_nb", &(m_tpc.m_timestamp_nb));
    m_tree->SetBranchAddress("timestamp_start", &(m_tpc.m_timestamp_start));
    m_tree->SetBranchAddress("timestamp_stop", &(m_tpc.m_timestamp_stop));

    m_tree->SetBranchAddress("m_Temperature", &(m_tpc.m_Temperature));
    m_tree->SetBranchAddress("m_Pressure12", &(m_tpc.m_Pressure1));
    m_tree->SetBranchAddress("m_Pressure34", &(m_tpc.m_Pressure2));
    m_tree->SetBranchAddress("m_Flow14", &(m_tpc.m_Flow1));
    //m_tree->SetBranchAddress("m_Flow2",&(m_tpc.m_Flow2));
    m_tree->SetBranchAddress("m_SetFlow", &(m_tpc.m_SetFlow));
    m_tree->SetBranchAddress("m_GetFlow", &(m_tpc.m_GetFlow));

    m_tree->SetBranchAddress("m_ILER", &(m_tpc.m_ILER));
    m_tree->SetBranchAddress("m_IHER", &(m_tpc.m_IHER));
    m_tree->SetBranchAddress("m_PLER", &(m_tpc.m_PLER));
    m_tree->SetBranchAddress("m_PHER", &(m_tpc.m_PHER));
    m_tree->SetBranchAddress("m_tauLER", &(m_tpc.m_tauLER));
    m_tree->SetBranchAddress("m_tauHER", &(m_tpc.m_tauHER));
    m_tree->SetBranchAddress("m_flagLER", &(m_tpc.m_flagLER));
    m_tree->SetBranchAddress("m_flagHER", &(m_tpc.m_flagHER));

    //m_tree->SetBranchAddress("runNum", &(m_tpc.runNum));
    //m_tree->SetBranchAddress("evtnb", &(m_tpc.eventNum));
    //m_tree->SetBranchAddress("eventflag", &(m_tpc.eventflag));
    //m_tree->SetBranchAddress("eventtag_tpc", &(m_tpc.eventtag_tpc));


    m_numEntries = m_tree->GetEntries();
    cout << "m_numEntries " << m_numEntries << endl;
    m_entryCounter = 0;

    // data store objects registration

    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.registerInDataStore();

    //StoreArray<MicrotpcDataHit> microtpcDataHits;
    //microtpcDataHits.registerInDataStore();
    StoreArray<MicrotpcDataHit>::registerPersistent();
    /*

    StoreArray<ExtHit> extHits;
    extHits.registerInDataStore();

    tracks.registerRelationTo(extHits);
    */
  }


  void TpcDataRootInputModule::beginRun()
  {
  }


  void TpcDataRootInputModule::event()
  {
    m_tpc.clear();

    // create data store objects

    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.create();

    if (m_eventCount == m_numEvents) {
      evtMetaData->setEndOfData(); // stpc event processing
      return;
    }

    StoreArray<MicrotpcDataHit> microtpcDataHits;

    //m_tree->GetEntry(m_entryCounter);
    m_tree->GetEntry(m_eventCount);

    //cout << " entry " << m_entryCounter << " total " << m_numEntries << endl;
    cout << " Evt counter " << m_eventCount << " total nb of Events " << m_numEvents << endl;
    for (int i = 0; i < (int)m_tpc.m_pxhits; i++) {
      int col = m_tpc.m_column[i];
      int row = m_tpc.m_row[i];
      int bcid = m_tpc.m_bcid[i];
      int tot = m_tpc.m_tot[i];
      int detNb = 3;
      int pdg = 0;
      int tkid = 0;
      //cout << "col " << col << " row " << row << " bcid " << bcid << " tot " << tot << endl;
      microtpcDataHits.appendNew(MicrotpcDataHit(col, row, bcid, tot, detNb, pdg, tkid));
    }
    //m_entryCounter++;
    m_eventCount++;
    // set event metadata
    evtMetaData->setEvent(m_tpc.eventNum);
    evtMetaData->setRun(m_tpc.runNum);


    /*
    B2INFO("run " << evtMetaData->getRun()
           << " event " << evtMetaData->getEvent()
     << " nhit=" << microtpcDataHits.getEntries()
           << " ps");
    */
  }


  void TpcDataRootInputModule::endRun()
  {
  }

  void TpcDataRootInputModule::terminate()
  {
    delete m_tree;
  }

  void TpcDataRootInputModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

