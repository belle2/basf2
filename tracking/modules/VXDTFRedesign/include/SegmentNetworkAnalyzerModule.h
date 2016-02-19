/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetworkContainer.h>
#include <tracking/trackFindingVXD/segmentNetwork/Segment.h>
#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/core/Module.h>

#include <TFile.h>
#include <TTree.h>

#include <string>

namespace Belle2 {
  /** class for analyzing the contents of the SegmentNetwork*/
  class SegmentNetworkAnalyzerModule : public Module {

  public:

    struct RootVariables {
      std::vector<double> phi{};
      std::vector<double> theta{};
      std::vector<double> pT{};
      std::vector<int> signal{};
      std::vector<int> pdg{};
      std::vector<int> virtualIP{};
      unsigned networkSize{};
      // std::vector<unsigned long> sectorComb;
    };

    SegmentNetworkAnalyzerModule();

    virtual void initialize();

    virtual void event();

    virtual void terminate();

  private:

    std::string m_PARAMnetworkName;

    std::string m_PARAMrootFileName;

    Belle2::StoreObjPtr<Belle2::DirectedNodeNetworkContainer> m_network;

    Belle2::StoreArray<Belle2::MCParticle> m_mcParticles;

    TFile* m_rFilePtr;

    TTree* m_treePtr;

    RootVariables m_rootVariables;

    void makeBranches();

    void analyzeCombination(const Belle2::Segment<Belle2::TrackNode>& outer, const Belle2::Segment<Belle2::TrackNode>& inner);
  };
}
