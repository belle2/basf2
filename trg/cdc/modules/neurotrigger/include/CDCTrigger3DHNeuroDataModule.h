/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/multi_array.hpp>

#include "trg/cdc/NeuroTrigger3DH.h"
#include "trg/cdc/dataobjects/CDCTriggerHoughMLP.h"
#include "trg/cdc/dataobjects/CDCTrigger3DHTrack.h"
#include "tracking/dataobjects/RecoTrack.h"
#include "framework/core/Module.h"

#define BOOST_MULTI_ARRAY_NO_GENERATORS

namespace Belle2 {
  // Class for the data generation for the network training data set using 3DHough Finder input tracks
  class CDCTrigger3DHNeuroDataModule : public Module {
  public:
    // Struct for target results
    struct TargetResult {
      std::vector<float> targetVector;
      unsigned short trackType;
    };
    // Enum for the (target) track type
    enum class TrackType : unsigned short {
      Real = 0,
      Background = 1,
      Fake = 2,
      UnrelatedFake = 3,
      Unknown = 4
    };

    // Construtor
    CDCTrigger3DHNeuroDataModule();
    // Destructor
    virtual ~CDCTrigger3DHNeuroDataModule() {}

    virtual void initialize() override;
    virtual void event() override;


  private:
    // Write the headline to the .gz file
    void writeHeadline() const;
    // Compute scaled target vector from reco track
    TargetResult computeTargetVector(const CDCTrigger3DHTrack& ndFinderTrack, const bool isFakeEvent) const;
    // Get the (target) track type
    TrackType determineTrackType(const float classificationNNT, const bool isFakeEvent, const bool isUnrelatedFake) const;

    // Data generation parameters
    // Name of the StoreArray containing the input track segment hits
    std::string m_hitCollectionName;
    // Name of the StoreArray containing the input 3D tracks
    std::string m_inputCollectionName;
    // Name of the StoreArray containing the reconstructed tracks used as target values
    std::string m_targetCollectionName;
    // Name of the configuration file used in the module to load the network configuration
    std::string m_configFileName;
    // Name of gzip file where the training data is saved
    std::string m_filename;
    // Flag to save the 3DFinder tracks from fake events (no reconstructed track present)
    bool m_saveFakeEventTracks;
    // Flag to save the 3DFinder tracks that have no relation to a reconstructed track
    bool m_saveFakeUnrelatedTracks;

    // Parameters for the 3DHough input NeuroTrigger
    NeuroParametersHough m_neuroParameters3DH;
    // Instance of the 3DHough input NeuroTrigger
    NeuroTrigger3DH m_neuroTrigger3DH;
    // StoreArray of input tracks
    StoreArray<CDCTrigger3DHTrack> m_ndFinderTracks;
    // StoreArray of reco tracks
    StoreArray<RecoTrack> m_recoTracks;

    // Number of super layers
    static constexpr size_t m_nSL = 9;
  };
}
