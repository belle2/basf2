/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef UPDATEPARTICLETRACKCANDMODULE_H
#define UPDATEPARTICLETRACKCANDMODULE_H

/* Belle2 headers. */
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

#endif /* UPDATEPARTICLETRACKCANDMODULE_H */
