/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class RecoTrack;
  class Track;
  class MCParticle;

  /** A module to set relations from Track to MCParticle via the RecoTrack the Track is related to.
   *  The functionality is: Take Track, get relation to RecoTrack, from RecoTrack get relation to
   *  MCParticle, and finally add a relation from Track to MCParticle.
   */
  class TrackToMCParticleRelatorModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    TrackToMCParticleRelatorModule();

    /** Destructor (empty). */
    ~TrackToMCParticleRelatorModule();

    /** Use this to initialize StoreArrays and relations.
     */
    void initialize() override;

    /** Called once for each event.
     */
    void event() override;
  private:
    StoreArray<Track>  m_Tracks;  /**< Tracks StoreArray */
    StoreArray<RecoTrack>  m_RecoTracks;  /**< RecoTracks StoreArray */
    StoreArray<MCParticle>  m_MCParticles;  /**< MCParticles StoreArray */

    std::string m_TracksName = "";  ///< Track StoreArray name (input).
    std::string m_RecoTracksName = "";   ///< RecoTrack StoreArray name (input).
    std::string m_MCParticlesName = "";   ///< MCParticle StoreArray name ().

  };
}
