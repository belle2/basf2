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

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/Shortcuts.h>
#include <tracking/trackFindingVXD/filterTools/MLRange.h>
#include <tracking/trackFindingVXD/filterMap/threeHitVariables/MLHandover.h>
#include <tracking/trackFindingVXD/filterTools/FBDTClassifier.h>

#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetworkContainer.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/core/Module.h>

// stl
#include <string>
#include <vector>
#include <memory>

#include <TFile.h>
#include <TTree.h>

namespace Belle2 {

  /** Segment network producer module with a Machine Learning classifier.
   *
   * This module should be an alternative to the SegmentNetworkProducerModule, however only for the three hit filter
   * step. The SegmentNetworkProducerModule has to be run before this, as it handles the creation of an intermediate
   * DirectedNodeNetwork that is then accessed by this Module to create the DirectedNodeNetwork< Segment >. To
   * function properly the three hit filters from the SegmentNetworkProducerModule have to be disabled.
   *
   * NOTE: This is no final solution but merely a "small" hack to check the feasibility of employing machine
   * learning techniques to the SectorMap approach
   */
  class MLSegmentNetworkProducerModule : public Module {

  public:

    MLSegmentNetworkProducerModule(); /**< module constructor */

    /** typedef with complete definition*/
    using MLFilter = Belle2::Filter<Belle2::MLHandover<Belle2::SpacePoint, 9>,
          Belle2::MLRange<Belle2::FBDTClassifier<9>, 9, double>,
          VoidObserver >;

    void initialize() override; /**< initialize module */

    void event() override; /**< event */

    void terminate() override; /**< terminate module */

  private:

    /** file where the FastBDT classifier is stored. */
    std::string m_PARAMfbdtFileName;

    /** name of the StoreObjPtr pointing to the network container used in this module. */
    std::string m_PARAMnetworkInputName;

    /** the name of the used SectorMap. */
    std::string m_PARAMsecMapName;

    /** name of the root file name */
    std::string m_PARAMrootFileName;

    /** cut value to be used with classifier */
    double m_PARAMcutVal;

    /** internal three hit filter*/
    std::unique_ptr<MLFilter> m_filter{nullptr};

    /** classifier used throughout this module */
    std::unique_ptr<Belle2::FBDTClassifier<9> > m_classifier{nullptr};

    /** StoreObjPtr to access the DNNs that are used in this module. */
    Belle2::StoreObjPtr<Belle2::DirectedNodeNetworkContainer> m_network;

    /** construct the classifier from file */
    void setupClassifier(const std::string& filename);

    /** setup the filter */
    void setupFilter();

  };

} // end namespace Belle2
