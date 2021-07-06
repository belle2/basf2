/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {

  /**
   * This class imports KLMLikelihoodParameters into the database.
   */
  class KLMLikelihoodParametersImporter {

  public:

    /**
     * Constructor
     */
    KLMLikelihoodParametersImporter() = default;

    /**
     * Destructor
     */
    ~KLMLikelihoodParametersImporter() = default;

    /**
     * Write KLMLikelihoodParameters into the database.
     */
    void writeLikelihoodParameters();

    /**
     * Read KLMLikelihoodParameters from the database.
     */
    void readLikelihoodParameters();

  };

}

