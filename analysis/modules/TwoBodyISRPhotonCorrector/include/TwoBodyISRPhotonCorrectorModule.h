/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//#pragma once
#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <string>

namespace Belle2 {

  /**  */

  class TwoBodyISRPhotonCorrectorModule : public Module {
  private:
    /** name of input particle list. */
    std::string m_gammaList;
    Int_t m_massiveParticle;

  public:
    /** Constructor. */
    TwoBodyISRPhotonCorrectorModule();

    /** Initialises the module.
     */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2


