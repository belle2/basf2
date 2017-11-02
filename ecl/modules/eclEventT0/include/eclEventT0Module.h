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
#include <framework/gearbox/Const.h>
#include <ecl/dataobjects/ECLCalDigit.h>

namespace Belle2 {
  /**
   * EventT0 from ECL
   *
   * EventT0 found using isolated eclCalDigits above 100 MeV. Results uploaded to framework/data/EventT0
   *
   */
  class eclEventT0Module : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    eclEventT0Module();

    /** Register input and output data */
    virtual void initialize();

    /** Event */
    virtual void event();


  private:

    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< eclCalDigits  */
    float m_isolationDr2 = 900.;  /**< minimum 30 cm between digits */
    float m_ethresh = 0.1; /**< minimum 100 MeV for CalDigit to be used */
    double m_stdDevNom = 20.; /**< nominal resolution (ns) for digit with E=m_ethresh */
    double m_stdDevLarge = 1000.; /**< report resolution (ns) if there is no information */
    /** ECL crystal locations */
    std::vector<float> m_xcrys;
    std::vector<float> m_ycrys;
    std::vector<float> m_zcrys;
    Const::EDetector m_eclID = Const::EDetector::ECL;
  };
}

