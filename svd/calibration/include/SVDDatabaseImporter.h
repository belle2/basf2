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
     * This method import to the database the neural network
     * for hit time determination
     * @param fileName is the file name of the xml file
     * @param threeSamples if true, calibrate with 3 samples?
     */

    void importSVDHitTimeNeuralNetwork(std::string fileName, bool threeSamples = false);


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
