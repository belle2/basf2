/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  class Particle;

  /**
   * Pseudo Vertex fitter module
   */
  class PseudoVertexFitterModule : public Module {

  public:

    /**
     * Constructor
     */
    PseudoVertexFitterModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    std::string m_listName;       /**< particle list name */
    std::string m_decayString;    /**< daughter particles selection */

    /**
    * Main steering routine
    * @param p pointer to particle
    * @return true for successfully adding the covariance matrix
    */
    bool add_matrix(Particle* p);
  };
} // Belle2 namespace

