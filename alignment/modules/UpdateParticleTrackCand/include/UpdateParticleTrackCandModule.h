/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * Updates the seed in TrackCand based on fitted state (at vertex)
   *
   * A TrackCand used to create the track for this particle (still using pion hypothesis) is updated (seed) and a special
   *
   */
  class UpdateParticleTrackCandModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    UpdateParticleTrackCandModule();

    /** init */
    virtual void initialize() override;

    /** update candidates */
    virtual void event() override;


  private:

    std::vector<std::string> m_motherListNames;  /**< Names of particle list which tracks should be updated */
    bool m_removePXD; /**< Remove PXD hits from TrackCand */
    bool m_removeSVD; /**< Remove SVD hits from TrackCand */
    bool m_removeCDC; /**< Remove CDC hits from TrackCand */
    bool m_removeBKLM; /**< Remove BKLM hits from TrackCand */
  };
}
