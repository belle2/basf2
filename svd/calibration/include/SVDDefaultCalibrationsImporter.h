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
   * This class import to the database the default values for the dbobjects
   * needed for the online calibrations.
   */
  class SVDDefaultCalibrationsImporter {

  public:

    /**
     * Default constructor
     */
    SVDDefaultCalibrationsImporter():
      m_firstExperiment(0), m_firstRun(0),
      m_lastExperiment(-1), m_lastRun(-1)
    {
    }

    /**
     * A constructor accepting as arguments the interval of
     * validity coordinates.
     */
    SVDDefaultCalibrationsImporter(int fexp, int frun, int lexp, int lrun):
      m_firstExperiment(fexp), m_firstRun(frun),
      m_lastExperiment(lexp), m_lastRun(lrun)
    {
    }


    /**
     * Destructor
     */
    virtual ~SVDDefaultCalibrationsImporter()
    {}

    /**
     * This method import to the database the default values for the constants
     * (charge, ADC counts, peaking time and width) needed for the calibration of the signal pulse shape.
     *
     */
    void importSVDPulseShapeCalibrations(/*std::string fileName*/);

    /**
     * This method import to the database the channel mapping which is always
     * required to properly fulfilled the other payloads.
     */

    void importSVDChannelMapping(const std::string& fileName = std::string("svd/data/svd_mapping.xml"));


    /**
         * This method import to the database the list of strips masked
         * at FADC level.
         */
    void importSVDFADCMaskedStrips();

    /**
     * This method import to the database the time shift correction for the
     * Cog calibration.
     */

    void importSVDTimeShiftCorrections(/*std::string fileName*/);

    /**
     * This method import to the database the strip
     * noise default values.
     *
     */
    void importSVDNoiseCalibrations();


    /**
     * This method import to the database the bad strip
     * status as flagged during the local runs.
     *
     *
     */
    void importSVDLocalRunBadStrips(/*std::string fileName*/);

    /**
     * This method imports SVD hit time estimator data to the database.
     * The data are in the form of an xml file created by the training
     * script.
     * The same is used for 3- and 6-sample networks.
     * @param filename name of the network definition xml to be imported.
     * @param threeSamples Attach label for 3-samples?
     */


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
    ClassDef(SVDDefaultCalibrationsImporter, 1);
  };

} // Belle2 namespace
