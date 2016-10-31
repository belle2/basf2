/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDATABASEIMPORTER_H
#define EKLMDATABASEIMPORTER_H

namespace Belle2 {

  /**
   * EKLM time calibration algorithm.
   */
  class EKLMDatabaseImporter {

  public:

    /**
     * Constructor.
     */
    EKLMDatabaseImporter();

    /**
     * Destructor.
     */
    ~EKLMDatabaseImporter();

    /**
     * Import digitization parameters.
     */
    void importDigitizationParameters();

    /**
     * Import simulation parameters.
     */
    void importSimulationParameters();

  };

}

#endif

