/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   * This class import to the database the dbobjects
   * SVDCoGTimeCalibrations.
   *
   */
  class SVDCoGTimeCalibrationsImporter {

  public:

    /**
     * Default constructor
     */
    SVDCoGTimeCalibrationsImporter():
      m_firstExperiment(0), m_firstRun(0),
      m_lastExperiment(-1), m_lastRun(-1)
    {
    }

    /**
     * A constructor accepting as arguments the interval of
     * validity coordinates.
     */
    SVDCoGTimeCalibrationsImporter(int fexp, int frun, int lexp, int lrun):
      m_firstExperiment(fexp), m_firstRun(frun),
      m_lastExperiment(lexp), m_lastRun(lrun)
    {
    }


    /**
     * Destructor
     */
    virtual ~SVDCoGTimeCalibrationsImporter()
    {}

    /**
     * This method import to the database the calibration constants
     * (charge, ADC counts, peaking time and width) from local
     * runs
     *
     * Currently they are not read from any file, they are fixed to
     * default values.
     */
    void importNULLCoGTimeCalibrations();

    void importCoGTimeCalibrations() {};

    /**********************************************/

    /**
     * Print the local-run calibration constants
     * (charge, ADC pulse, peaking time and width).
     * Input: none
     * Output: none
     */
    void printSVDCoGTimeCalibrations(int layer, int ladder, int sensor, bool isUside);
  private:

    /**
     * The interval of validity coordinates are defined as
     * private members.
     */
    int m_firstExperiment; /**< First experiment. */
    int m_firstRun; /**< First run. */
    int m_lastExperiment; /**< Last experiment */
    int m_lastRun; /**< Last run. */
    ClassDef(SVDCoGTimeCalibrationsImporter, 1);
  };

} // Belle2 namespace
