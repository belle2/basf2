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

    /** keep all the variables for rootoutput in one struct */
    struct RootVariables {
      std::vector<double> phi{}; /** phi of the innermost hit (that is not the virtual IP) */
      std::vector<double> theta{}; /** theta of the innermost hit (that is not the virtual IP) */
      std::vector<double> pT{}; /** pT of the related MCParticle */
      std::vector<int> signal{}; /** was segment combination signal */
      std::vector<int> passed{}; /** did segment combination pass the three hit filters */
      std::vector<int> pdg{}; /** pdg of the related MCParticle */
      std::vector<int> virtualIP{}; /** did the segment contain the virtual IP */
      unsigned networkSize{}; /** segmentNetwork size */
      unsigned networkConnections{}; /** number of connections in network */
      // std::vector<unsigned long> sectorComb;
    };

    /** constructor */
    SegmentNetworkAnalyzerModule();

    /** set up root file and check required Store Arrays */
    void initialize() override;

    /** collect necessary data and put into TTree */
    void event() override;

    /** write and close root file*/
    void terminate() override;

  private:

    /** StoreArray name of the DirectedNodeNetworkContainer */
    std::string m_PARAMnetworkName;

    /** file name of the produced root file */
    std::string m_PARAMrootFileName;

    /** StoreObjPtr to the SegmentNetwork and TrackNode Network container */
    Belle2::StoreObjPtr<Belle2::DirectedNodeNetworkContainer> m_network;

    /** MCParticles StoreArray for obtaining MC Information */
    Belle2::StoreArray<Belle2::MCParticle> m_mcParticles;

    /** ptr to root file */
    TFile* m_rFilePtr;

    /** ptr to TTree */
    TTree* m_treePtr;

    /** handle to collect all data for one event */
    RootVariables m_rootVariables;

    /** setup the Branches in the output TTree */
    void makeBranches();

    /** get necessary data from three hit combination and put them into the root variables */
    void analyzeCombination(const Belle2::Segment<Belle2::TrackNode>& outer, const Belle2::Segment<Belle2::TrackNode>& inner,
                            bool passed);

    /** get the number of connections between the nodes of a network */
    template<typename EntryType, typename MetaInfoType>
    size_t getNConnections(Belle2::DirectedNodeNetwork<EntryType, MetaInfoType>& network) const;

  };
}
