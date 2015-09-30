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

    SecMapTrainDataSet m_data;

    TFile* m_file;

    std::string m_name;

    /** interface to the TTree. */
    StoreObjPtr<RootMergeable<TTree>> m_tree;

  public:

    /** Constructor - prepares ttree. */
    RawSecMapRootInterface(TFile* file, std::string mapName) : m_file(file), m_name(mapName)
    {
      m_file->cd();
      m_tree.registerInDataStore(mapName, DataStore::c_Persistent);
      if (!m_tree.isValid()) {
        m_tree.construct(mapName.c_str(), "Raw data for a sectorMap");
      }
      m_tree->get().Branch("expNo", &m_data.expNo);
      m_tree->get().Branch("runNo", &m_data.runNo);
      m_tree->get().Branch("evtNo", &m_data.evtNo);
      m_tree->get().Branch("trackNo", &m_data.trackNo);
      m_tree->get().Branch("pdg", &m_data.pdg);
      m_tree->get().Branch("filter", &m_data.filterID);
      m_tree->get().Branch("secIDChain", &m_data.sectorIDs);
      m_tree->get().Branch("value", &m_data.value);
      m_tree->get().SetDirectory(nullptr);
    }

    void fill(const SecMapTrainDataSet& newSet)
    {
      m_data = newSet;
      m_tree->get().Fill();
      m_file->cd();
    }

    /** write to file at end of processing. */
    void write()
    {
      if (!m_file || !m_file->IsOpen()) {
        B2ERROR("RawSecMapRootInterface-write: for map " << m_name <<
                ": file is not accessable and thererore no writing could be done!")
        return;
      }
//    m_file->Open();
      m_file->cd();
      m_tree->write(m_file);
      m_tree->get().SetDirectory(nullptr);
      m_file->Close();
    }

  };
}

