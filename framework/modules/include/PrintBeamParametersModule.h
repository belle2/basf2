/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/dbobjects/BeamParameters.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {
  /**
   * Print the BeamParameters everytime they change
   *
   *    *
   */
  class PrintBeamParametersModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PrintBeamParametersModule();

    /** print the Beam Parameters if they changed */
    void event();

  private:
    /** print a covariance matrix and simplify it to three elements if the off diagonals are zero */
    static void printCovMatrix(std::ostream& out, const TMatrixDSym& cov);

    /** Pointer to the BeamParameters */
    DBObjPtr<BeamParameters> m_beamparams;
  };
}
