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

    void importSVDChannelMapping(const std::string& fileName = std::string("svd/data/svd_mapping.xml"));

    void importSVDTimeShiftCorrections(/*std::string fileName*/);

    /**
     * This method import to the database the strip
     * noise from the noise local runs.
     *
     * Currently they are not read from any file, they are fixed to
     * default values.
     */
    void importSVDNoiseCalibrations();

    /**
     * This method import to the database the strip
     * noise from the noise local runs.
     * @param xmlFileName is the file name of the xml file
     * @param errorTollerant if true bypasses sanity checks
     */
    void importSVDNoiseCalibrationsFromXML(const std::string& xmlFileName, bool errorTollerant = false);
    /**
     * This method import to the database the strip
     * pedestal from the pedestal local runs.
     * @param xmlFileName is the file name of the xml file
     * @param errorTollerant if true bypasses sanity checks
     */
    void importSVDPedestalCalibrationsFromXML(const std::string& xmlFileName, bool errorTollerant = false);
    /**
     * This method import to the database the strip pulse
     * width, time and gain from the injection local runs.
     * @param xmlFileName is the file name of the xml file
     * @param errorTollerant if true bypasses sanity checks
     */

    void importSVDCalAmpCalibrationsFromXML(const std::string& xmlFileName, bool errorTollerant = false);

    /**
     * This method import to the database the strip
     * noise from the noise local runs.
     * @param condDbname is the name of the payload in the condition database
     * @param xmlFileName is the file name of the xml file
     * @param xmlTag is the tag in the xml file that contains the actual calibrations
     * @param default is the value assigned to the calibration constant by default
     * @param errorTollerant if true bypasses sanity checks
     */

    template< class SVDcalibration >
    void importSVDCalibrationsFromXML(const std::string& condDbname,
                                      const std::string& xmlFileName,
                                      const std::string& xmlTag,
                                      typename SVDcalibration::t_perSideContainer::calibrationType defaultValue,
                                      bool errorTollerant);

    /**
     * This method import to the database the bad strip
     * status as flagged during the local runs.
     *
     * Currently they are not read from any file, return
     * false as default value.
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
    void importSVDHitTimeNeuralNetwork(std::string fileName, bool threeSamples = false);

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
