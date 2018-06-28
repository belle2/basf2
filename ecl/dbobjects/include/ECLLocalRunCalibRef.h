/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * ECLLocalRunCalibRef                                                    *
 *                                                                        *
 * This class is designed to store reference marks to database for        *
 * ECL local run calibration.                                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef _ECLLocalRunCalibRef_H_
#define _ECLLocalRunCalibRef_H_
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
#endif
