/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: czhearty                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <framework/dataobjects/EventT0.h>

namespace Belle2 {
  namespace ECL {

    /**
     * EventT0 from ECL
     *
     * EventT0 found using isolated ECLCalDigits above 100 MeV. Results uploaded to
     * framework/dataobjects/EventT0
     * Description of the algorithm at https://agira.desy.de/browse/BII-2561
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
      double m_isolationDr2;  /**< minimum 30 cm between digits */
      double m_ethresh; /**< minimum 100 MeV for CalDigit to be used */
      double m_stdDevNom; /**< nominal resolution (ns) for digit with E=m_ethresh */
      double m_stdDevLarge; /**< report resolution (ns) if there is no information */
      std::vector<float> m_xcrys; /**< ECL crystal location X */
      std::vector<float> m_ycrys; /**< ECL crystal location Y */
      std::vector<float> m_zcrys; /**< ECL crystal location Z */
    };
  }
}

