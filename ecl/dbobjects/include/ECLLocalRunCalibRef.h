/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

// ROOT
#include <TObject.h>
namespace Belle2 {
  /**
   * ECLLocalRunCalibRef is designed
   * to store reference marks to database
   * for ECL local run calibration.
   */
  class ECLLocalRunCalibRef : public TObject {
  public:
    /**
     * Constructor.
     */
    ECLLocalRunCalibRef();
    /**
     * Constructor.
     * @param curExp is the experiment number.
     * @param curRun is the run number.
     */
    ECLLocalRunCalibRef(const int& curExp,
                        const int& curRun);
    /**
     * Destructor.
     */
    ~ECLLocalRunCalibRef();
    /**
     * Get experiment number.
     */
    int getExp() const;
    /**
     * Get run number.
     */
    int getRun() const;
  private:
    /**
     * Experiment number.
     */
    const int c_exp;
    /**
     * Run number.
     */
    const int c_run;
    ClassDef(ECLLocalRunCalibRef, 1);
  };
}

