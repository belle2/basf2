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

#include <tracking/trackFindingVXD/filterTools/FBDTClassifierHelper.h> // for the FBDTTrainSample
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetworkContainer.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <array>
#include <vector>
#include <iostream>

namespace Belle2 {


  /** Module for collecting the data and training a FastBDT classifier.
   *
   * Uses the TrackNode network that is created by the SegmentNetworkProducerModule as input to collect all three
   * hit combinations that are allowed by it and collects them to train a FBDTClassifier in the end and store the
   * result to a file.
   */
  class FastBDTClassifierTrainingModule : public Module {

  public:

    /** module constructor. */
    FastBDTClassifierTrainingModule();

    /** initialize the module */
    void initialize() override;

    /** collect all possible combinations and store them */
    void event() override;

    /** take the collected data and train a FBDTClassifier and store it in the given output file */
    void terminate() override;

  private:

    /**< private typedef for shorter notation */
    using TrainSample = Belle2::FBDTTrainSample<9>;

    /** output file name into which the FBDTClassifier is stored. */
    std::string m_PARAMfbdtOutFileName;

    /** name of the StoreObjPtr in which the network container is stored which contains the network that is used to create the three hit samples. */
    std::string m_PARAMnetworkInputName;

    /** filename to be used to store / read collect samples */
    std::string m_PARAMsamplesFileName;

    /** actually train a classifier or only do collection */
    bool m_PARAMdoTrain;

    /** store the collected samples into a file */
    bool m_PARAMstoreSamples;

    /** use pre-collected samples for training and bypass the collection step */
    bool m_PARAMuseSamples;

    /** number of trees in the FastBDT. */
    int m_PARAMnTrees;

    /** tree depth in FastBDT. */
    int m_PARAMtreeDepth;

    /** shrinkage parameter of FastBDT. */
    double m_PARAMshrinkage;

    /** ratio of samples to be used for training one tree in the FastBDT. */
    double m_PARAMrandRatio;

    /** vector in which all samples are collected on the fly in event. */
    std::vector<TrainSample> m_samples;

    /** StoreObjPtr to access the DNNs that are used in this module. */
    Belle2::StoreObjPtr<Belle2::DirectedNodeNetworkContainer> m_network;

    /** create a trainings sample from the three hit combination */
    const TrainSample makeTrainSample(const Belle2::SpacePoint* outerHit, const Belle2::SpacePoint* centerHit,
                                      const Belle2::SpacePoint* innerHit);
  };

} // end namespace Belle2
