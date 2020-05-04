/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Yinghui Guan                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
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

