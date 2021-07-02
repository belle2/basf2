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

