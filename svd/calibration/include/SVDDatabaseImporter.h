/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani                                               *
 *                                                                       *
 * This software is provided "as is" without any warranty.                *
 * WARNING: Do not try to fry it with water. Use only olive oil.          *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   * This class import to the database the dbobjects
   * SVDNoiseCalibrations and SVDPulseShapeCalibrations.
   *
   * Currently, no input file is read to fill them, default
   *  values are written on the dbobjects for the needed calibrations
   * constants as noise, charge, ADC pulse, peaking time and
   * pulse width.
   */
  class SVDDatabaseImporter {

  public:

    /**
     * Default constructor
     */
    SVDDatabaseImporter():
      m_firstExperiment(0), m_firstRun(0),
      m_lastExperiment(-1), m_lastRun(-1)
    {
    }

    /**
     * A constructor accepting as arguments the interval of
     * validity coordinates.
     */
    SVDDatabaseImporter(int fexp, int frun, int lexp, int lrun):
      m_firstExperiment(fexp), m_firstRun(frun),
      m_lastExperiment(lexp), m_lastRun(lrun)
    {
    }


    /**
     * Destructor
     */
    virtual ~SVDDatabaseImporter()
    {}

    /**
     * This method import to the database the calibration constants
     * (charge, ADC counts, peaking time and width) from local
     * runs
     *
     * Currently they are not read from any file, they are fixed to
     * default values.
     */
    void importSVDPulseShapeCalibrations(/*std::string fileName*/);

    void importSVDTimeShiftCorrections(/*std::string fileName*/);

    /**
     * This method import to the database the strip
     * noise from the noise local runs.
     *
     * Currently they are not read from any file, they are fixed to
     * default values.
     */
    void importSVDNoiseCalibrations(/*std::string fileName*/);

    /**
     * This method import to the database the bad strip
     * status as flagged during the local runs.
     *
     * Currently they are not read from any file, return
     * false as default value.
     *
     */
    void importSVDLocalRunBadStrips(/*std::string fileName*/);

    /**********************************************/

    /**
     * Print the local-run calibration constants
     * (charge, ADC pulse, peaking time and width).
     * Input: none
     * Output: none
     */
    void printSVDPulseShapeCalibrations();

    /**
     * Print the strip noise.
     * Input: none
     * Output: none
     */
    void printSVDNoiseCalibrations();


    /**
     * Print the status of a given strip,
     * return 1 if the strip is flagged as a bad one.
     * Input: none
     * Output: none
     */
    void printSVDLocalRunBadStrips();

  private:

    /**
     * The interval of validity coordinates are defined as
     * private members.
     */
    int m_firstExperiment; /**< First experiment. */
    int m_firstRun; /**< First run. */
    int m_lastExperiment; /**< Last experiment */
    int m_lastRun; /**< Last run. */
    ClassDef(SVDDatabaseImporter, 1);
  };

} // Belle2 namespace
