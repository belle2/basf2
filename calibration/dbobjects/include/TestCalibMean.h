/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TH1I.h>

namespace Belle2 {

  /**
   *   Test DBObject inheriting from TH1I
   */

  class TestCalibMean: public TH1I {

  public:

    /**
     * Default constructor
     */
    TestCalibMean() {};

    /**
     * Constructor
     */
    TestCalibMean(std::string name, int mean): TH1I(name.c_str(), "TestCalibMean", 100, 0, 100) {this->Fill(mean);};

    /**
     * Destructor
     */
    ~TestCalibMean() {};
    // Could extend this class with more methods to access/calculate things.
    // Also we still have access to TH1I's methods such as ->GetMean(), ->Print(), even ->Draw()...

    ClassDef(TestCalibMean, 1); /**< ClassDef */
  };
} // end namespace Belle2
