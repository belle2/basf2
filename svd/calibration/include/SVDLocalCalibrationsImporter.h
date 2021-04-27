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
   * This class import to the database the dbobjects storing the
   * SVD Local Calibrations.
   *
   */
  class SVDLocalCalibrationsImporter {

  public:

    /**
     * Default constructor
     */
    SVDLocalCalibrationsImporter():
      m_firstExperiment(0), m_firstRun(0),
      m_lastExperiment(-1), m_lastRun(-1)
    {
    }

    /**
     * A constructor accepting as arguments the interval of
     * validity coordinates.
     */
    SVDLocalCalibrationsImporter(int fexp, int frun, int lexp, int lrun):
      m_firstExperiment(fexp), m_firstRun(frun),
      m_lastExperiment(lexp), m_lastRun(lrun)
    {
    }


    /**
     * Destructor
     */
    virtual ~SVDLocalCalibrationsImporter()
    {}
    /**
         * This method import to the database the channel mapping which is always
         * required to properly fulfilled the other payloads.
         */

    void importSVDChannelMapping(const std::string& fileName = std::string("svd/data/svd_mapping.xml"));



    /**
     * This method import to the database the strip
     * noise from the pedestal local runs.
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
     * This method import to the database the list of hot
     * strips flagged online and written in the local run xml output.
     * @param xmlFileName is the file name of the xml file
     * @param errorTollerant if true bypasses sanity checks
     */
    void importSVDHotStripsCalibrationsFromXML(const std::string& xmlFileName, bool errorTollerant = false);


    /**
         * This method import to the database the list of strips
         * masked at FADC level.
         * @param xmlFileName is the file name of the xml file
         * @param errorTollerant if true bypasses sanity checks
         */
    void importSVDFADCMaskedStripsFromXML(const std::string& xmlFileName, bool errorTollerant = false);


    /**
     * This method import to the database the strip
     * noise from the noise local runs.
     * @param condDbname is the name of the payload in the condition database
     * @param xmlFileName is the file name of the xml file
     * @param xmlTag is the tag in the xml file that contains the actual calibrations
     * @param defaultValue is the value assigned to the calibration constant by default
     * @param errorTollerant if true bypasses sanity checks
     */

    template< class SVDcalibration >
    void importSVDCalibrationsFromXML(const std::string& condDbname,
                                      const std::string& xmlFileName,
                                      const std::string& xmlTag,
                                      typename SVDcalibration::t_perSideContainer::calibrationType defaultValue,
                                      bool errorTollerant);


  private:

    /**
     * The interval of validity coordinates are defined as
     * private members.
     */
    int m_firstExperiment; /**< First experiment. */
    int m_firstRun; /**< First run. */
    int m_lastExperiment; /**< Last experiment */
    int m_lastRun; /**< Last run. */

  };

} // Belle2 namespace
