/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/sectorMapTools/SecMapTrainDataSet.h>

#include <framework/logging/Logger.h>

#include <framework/pcore/RootMergeable.h>
#include <framework/datastore/StoreObjPtr.h>


#include <TFile.h>
#include <TList.h>
#include <TTree.h>
#include <string>


namespace Belle2 {

  /** To be used as an interface to root-stuff. */
//   template<class DataType, class TreeType>
  class RawSecMapRootInterface {
  protected:

    /** Mask for storing datasets to be piped into Trees. */
    SecMapTrainDataSet m_data;

    /** a pointer to the file where the Tree shall be stored. */
    TFile* m_file;
    TFile* m_file2; // WARNING temporary workaround for being able to fill ttress (not multithreading-compatible!)

    /** name of the StoreObjPtr. */
    std::string m_name;

    /** interface to the TTree. */
//     StoreObjPtr<RootMergeable<TTree>> m_tree; // TODO
    TTree* m_testTree;

  public:

    /** Constructor - prepares ttree. */
    RawSecMapRootInterface(TFile* file, std::string mapName) : m_file(file), m_name(mapName), m_testTree(nullptr)
    {
      /// WARNING: StoreObjPtr< RootMergeable< TTree > > m_tree does not work yet, has to be finished a.s.a.p.!
//       m_file->cd();
//       m_tree.registerInDataStore(mapName, DataStore::c_Persistent);
//       if (!m_tree.isValid()) {
//         m_tree.construct(mapName.c_str(), "Raw data for a sectorMap");
//       }
//       m_tree->get().Branch("expNo", &m_data.expNo);
//       m_tree->get().Branch("runNo", &m_data.runNo);
//       m_tree->get().Branch("evtNo", &m_data.evtNo);
//       m_tree->get().Branch("trackNo", &m_data.trackNo);
//       m_tree->get().Branch("pdg", &m_data.pdg);
//       m_tree->get().Branch("filter", &m_data.filterID);
//       m_tree->get().Branch("secIDChain", &m_data.sectorIDs);
//       m_tree->get().Branch("value", &m_data.value);
//       m_tree->get().SetDirectory(nullptr);
    }


    /** initialize the RawSecMapRootInterface (to be called in Module::initialize(). */
    void initialize()
    {
      B2DEBUG(1, "RawSecMapRootInterface::initialize: start")
      /// WARNING: StoreObjPtr< RootMergeable< TTree > > m_tree does not work yet, has to be finished a.s.a.p.!
//    m_file->cd();
//    bool registered = m_tree.registerInDataStore(m_name, DataStore::c_Persistent);
//    bool constructed = m_tree.construct(m_name.c_str(), "Raw data for a sectorMap");
//    B2DEBUG(1, "RawSecMapRootInterface::initialize: isRegistered/isConstructed: " << registered << "/" << constructed)
//
//    m_tree->get().Branch("expNo", &m_data.expNo);
//    m_tree->get().Branch("runNo", &m_data.runNo);
//    m_tree->get().Branch("evtNo", &m_data.evtNo);
//    m_tree->get().Branch("trackNo", &m_data.trackNo);
//    m_tree->get().Branch("pdg", &m_data.pdg);
//    m_tree->get().Branch("filter", &m_data.filterID);
//    m_tree->get().Branch("secIDChain", &m_data.sectorIDs);
//    m_tree->get().Branch("value", &m_data.value);
//    B2DEBUG(1, "RawSecMapRootInterface::initialize: nBranches/nEntries: " << m_tree->get().GetNbranches () << "/" << m_tree->get().GetEntries())
//    m_tree->get().Fill();
//    m_file->Write();
//    m_tree->get().SetDirectory(nullptr);

      m_file2 = new TFile("noMergeableTest.root", "RECREATE");

      m_testTree = new TTree(m_name.c_str(), "Raw data for a sectorMap");
      m_testTree->Branch("expNo", &m_data.expNo);
      m_testTree->Branch("runNo", &m_data.runNo);
      m_testTree->Branch("evtNo", &m_data.evtNo);
      m_testTree->Branch("trackNo", &m_data.trackNo);
      m_testTree->Branch("pdg", &m_data.pdg);
      m_testTree->Branch("filter", &m_data.filterID);
      m_testTree->Branch("secIDChain", &m_data.sectorIDs);
      m_testTree->Branch("value", &m_data.value);
      m_testTree->Fill();
      m_file2->Write();
      B2DEBUG(1, "RawSecMapRootInterface::initialize: nBranches/nEntries: " << m_testTree->GetNbranches() << "/" <<
              m_testTree->GetEntries())
    }


    /** fill given dataSet into root Tree. */
    void fill(const SecMapTrainDataSet& newSet)
    {
      m_data = newSet;
      /// WARNING: StoreObjPtr< RootMergeable< TTree > > m_tree does not work yet, has to be finished a.s.a.p.!
//    if (!m_tree.isValid()) {
//    B2ERROR("RawSecMapRootInterface-fill: StoreObjPtr not valid! You probably forgot to call RawSecMapRootInterface.initialize() in Module::initialize!")
//    return;
//    }
//    m_file->cd();
//       m_tree->get().Fill();

      m_file2->cd();
      m_testTree->Fill();
    }


    /** write to file at end of processing. */
    void write()
    {
      /// WARNING: StoreObjPtr< RootMergeable< TTree > > m_tree does not work yet, has to be finished a.s.a.p.!
//    B2DEBUG(1, "RawSecMapRootInterface::write: start")
//       if (!m_file || !m_file->IsOpen()) {
//         B2ERROR("RawSecMapRootInterface-write: for map " << m_name <<
//                 ": file is not accessable and thererore no writing could be done!")
//         return;
//       }
//       m_file->cd();
//       B2DEBUG(1, "RawSecMapRootInterface::write: nBranches/nEntries: " << m_tree->get().GetNbranches()) // << "/" << m_tree->get().GetEntriesFast())
//       m_tree->write(m_file);
// //       m_tree->get().SetDirectory(nullptr);
//       m_file->Close();
//
      m_file2->cd();
      B2DEBUG(1, "RawSecMapRootInterface::testwrite: nBranches/nEntries: " << m_testTree->GetNbranches() << "/" <<
              m_testTree->GetEntriesFast())
      m_testTree->Print();
      m_file2->Write();
      m_file2->Close();
      B2DEBUG(1, "RawSecMapRootInterface::write: end")
    }

  };
}

