/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "trg/cdc/NeuroTrigger3DH.h"
#include "trg/cdc/dataobjects/CDCTriggerSegmentHit.h"
#include "trg/cdc/dataobjects/CDCTrigger3DHTrack.h"
#include "framework/core/Module.h"
#include "framework/datastore/StoreArray.h"

namespace Belle2 {
  /*
    The neural network module of the CDC trigger for 3DFinder track candidates.
    For binary extended input deep neural networks with classification node.
  */
  class CDCTrigger3DHNeuroModule : public Module {
  public:
    // Constructor
    CDCTrigger3DHNeuroModule();
    // Destructor
    virtual ~CDCTrigger3DHNeuroModule() {}

    // Module initialization (load network from root file, set data stores)
    virtual void initialize() override;
    // Run the neural network for each event
    virtual void event() override;

  private:
    // Get the quadrant number for a 3DFinder track
    short getQuadrant(const int trackIdx);
    // Name of file where network weights etc. are stored.
    std::string m_fileName;
    // Name of the TObjArray holding the networks.
    std::string m_arrayName;
    // Name of the StoreArray containing the input track segment hits.
    std::string m_hitCollectionName;
    // Name of the StoreArray containing the input 3DFinder tracks.
    std::string m_inputCollectionName;
    // Name of the StoreArray containing the resulting Neuro tracks.
    std::string m_outputCollectionName;
    // Switch to execute the network with fixed point calculation.
    bool m_fixedPoint;
    // Global classification cut to set nntBit (y-Bit)
    double m_classificationCutNNT;
    // Global classification cut to set sttBit
    double m_classificationCutSTT;

    // StoreArray of input 3DFinder tracks
    StoreArray<CDCTrigger3DHTrack> m_ndFinderTracks;
    // StoreArray of input track segment hits
    StoreArray<CDCTriggerSegmentHit> m_trackSegmentHits;
    // StoreArray of ouptput Neuro3D tracks
    StoreArray<CDCTrigger3DHTrack> m_neuro3DHTracks;

    // Instance of the NeuroTrigger3DH
    NeuroTrigger3DH m_neuroTrigger3DH;
    // Instance of the MLP from the conditions database
    DBObjPtr<CDCTrigger3DHMLP> m_CDCTrigger3DHMLPConditionsDB;
  };
}
