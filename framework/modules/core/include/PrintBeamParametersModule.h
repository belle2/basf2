/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    void event() override;

  private:
    /** print a covariance matrix and simplify it to three elements if the off diagonals are zero */
    static void printCovMatrix(std::ostream& out, const TMatrixDSym& cov);

    /** Pointer to the BeamParameters */
    DBObjPtr<BeamParameters> m_beamparams;
  };
}
