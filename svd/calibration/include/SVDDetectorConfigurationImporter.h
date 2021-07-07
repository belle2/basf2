/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   * This class import to the database the dbobjects
   * SVDGlobalConfigParameters and SVDLocalConfigParameters.
   *
   *
   */
  class SVDDetectorConfigurationImporter {

  public:

    /**
     * Default constructor
     */
    SVDDetectorConfigurationImporter():
      m_firstExperiment(0), m_firstRun(0),
      m_lastExperiment(-1), m_lastRun(-1)
    {
    }

    /**
     * A constructor accepting as arguments the interval of
     * validity coordinates.
     */
    SVDDetectorConfigurationImporter(int fexp, int frun, int lexp, int lrun):
      m_firstExperiment(fexp), m_firstRun(frun),
      m_lastExperiment(lexp), m_lastRun(lrun)
    {
    }


    /**
     * Destructor
     */
    virtual ~SVDDetectorConfigurationImporter()
    {}

    /**
     * This method import to the database the global configuration xml file
     * used during data taking
     */

    void importSVDGlobalXMLFile(const std::string& fileName = std::string("svd/data/global.xml"));


    /**
     * This method import to the database the global configuration parameters
     * used during data taking
     * @param xmlFileName is the file name of the xml file
     */

    void importSVDGlobalConfigParametersFromXML(const std::string& xmlFileName);

    /**
     * This method import to the database the local configuration parameters
     * used during data taking
     * @param xmlFileName is the file name of the xml file
     */

    void importSVDLocalConfigParametersFromXML(const std::string& xmlFileName);

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
