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
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  class EventT0;
  class ECLCalDigit;

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
    virtual void initialize() override;

    /** Event */
    virtual void event() override;


  private:

    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< eclCalDigits  */
    StoreObjPtr<EventT0> m_eventT0; /**< StoreObj EventT0  */
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
