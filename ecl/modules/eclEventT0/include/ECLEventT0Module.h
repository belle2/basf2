/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty hearty@physics.ubc.ca                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <framework/dataobjects/EventT0.h>

namespace Belle2 {

  /**
   * EventT0 from ECL
   *
   * EventT0's found using ECLCalDigits above threshold. Results uploaded to
   * framework/dataobjects/EventT0.
   * Code sets all possible event T0 values as TemporaryEventT0's;
   * One of them, as specified by m_primaryT0, is set as EventT0.
   * Description of the algorithm at https://agira.desy.de/browse/BII-2561
   * and in BELLE2-NOTE-TE-2017-017
   */
  class ECLEventT0Module : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLEventT0Module();

    /** Register input and output data */
    virtual void initialize();

    /** Event */
    virtual void event();


  private:

    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< eclCalDigits  */
    double m_ethresh; /**< minimum energy for ECLCalDigit to be used (0.06 GeV) */
    double m_maxDigitT; /**< maximum absolute value of ECLCalDigit time to be used (150 ns) */
    double m_sigmaScale; /**< scale factor for time resolution (ie dt99) of ECLCalDigit (0.15) */
    double m_maxT0; /**< maximum allowed absolute value of T0, in ns (135 ns)  */
    double m_T0bin; /**< step size between T0 hypotheses (1 ns)*/
    int m_primaryT0;/**< select which T0 is primary, ie first one reported. (0) */
    /* 0 = lowest chi square among local minima
     * 1 = absolute lowest chi square
     * 2 = closest to 0 */

  };
}

