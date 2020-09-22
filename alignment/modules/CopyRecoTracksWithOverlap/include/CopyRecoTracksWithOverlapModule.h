/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  class RecoTrack;

  /// Copy RecoTracks with overlap hits in VXD to a new StoreArray (Will need a refit).
  /// If particleList is specified, take only RecoTracks associated to the particles (allows to make prior cuts at analysis level).
  class CopyRecoTracksWithOverlapModule : public Module {

  public:

    /// Constructor: Sets the description, the properties and the parameters of the module.
    CopyRecoTracksWithOverlapModule();

    /// init
    virtual void initialize() override;

    /// Copy RecoTracks with overlaps (for particle if specified)
    virtual void event() override;

  private:
    /// Check if RecoTrack has overlap hits -> if yes, copy to a new array
    void processRecoTrack(const RecoTrack& track);

    /// Name of StoreArray with output RecoTracks with overlaps
    std::string m_overlapRecoTracksArrayName{"RecoTracksWithOverlap"};

    /// Name of particle list for which associated RecoTracks should be copied
    std::string m_particleListName{""};

    /// Tracks.
    StoreArray<RecoTrack> m_RecoTracks;

    /// Overlapping tracks.
    StoreArray<RecoTrack> m_OverlappingRecoTracks;

    /// Particle list.
    StoreObjPtr<ParticleList> m_ParticleList;

  };
}
