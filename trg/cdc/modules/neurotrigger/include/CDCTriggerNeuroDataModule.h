/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <trg/cdc/NeuroTrigger.h>
#include <trg/cdc/NeuroTriggerParameters.h>
#include <trg/cdc/dataobjects/CDCTriggerMLPData.h>
#include <iostream>
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/multi_array.hpp"
#define BOOST_MULTI_ARRAY_NO_GENERATORS
namespace Belle2 {
  /**
   */
  class CDCTriggerNeuroDataModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    CDCTriggerNeuroDataModule();

    /** Destructor. */
    virtual ~CDCTriggerNeuroDataModule() {}

    /** Initialize the module.
     * Initialize the networks and register datastore objects.
     */
    virtual void initialize() override;

    /** Called once for each event.
     * Prepare input and target for each track and store it.
     */
    virtual void event() override;

    /** Do the training for all sectors. */
    virtual void terminate() override;

    bool loadIDHist(const std::string& filename, NeuroTrigger& neuro);

    void saveData_gzip(const std::string& filename, const std::vector<CDCTriggerMLPData>& dataset,
                       boost::iostreams::filtering_streambuf<boost::iostreams::output> outStream);

  protected:
    //module parameters
    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;
    /** Name of the StoreArray containing the input 2D tracks. */
    std::string m_inputCollectionName;
    /** Switch between MCParticles or RecoTracks as targets. */
    bool m_trainOnRecoTracks;
    /** Name of the MCParticles/RecoTracks collection used as target values. */
    std::string m_targetCollectionName;
    /** name of the event time StoreObjPtr */
    std::string m_EventTimeName;
    /** Name of gzip file where the training data are saved. */
    std::string m_filename;
    /** Name of the configuration file used in the module to load the neuroparamters. */
    std::string m_configFileName;
    /** Name of the configuration file used in the module to write the neuroparamters. */
    std::string m_writeconfigFileName;
    /** Parameters for the NeuroTrigger. */
    NeuroTriggerParameters m_neuroParameters;
    /** Switch to rescale out of range target values or ignore them. */
    bool m_neuroTrackInputMode;
    /** counter for tracks */
    int m_trackcounter;
    //other data
    /** Instance of the NeuroTrigger. */
    NeuroTrigger m_NeuroTrigger;
    /** Sets of training data for all sectors. */
    std::vector<CDCTriggerMLPData> m_trainSet;

    /** List of input tracks. */
    StoreArray<CDCTriggerTrack> m_tracks;
    /** use a track only once and not for every expert */
    bool m_singleUse;
  };
}
