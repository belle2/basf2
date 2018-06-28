/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * ECLLocalRunCalibLoader                                                 *
 *                                                                        *
 * ECLLocalRunCalibLoader is designed to load tree of the local           *
 * run calibration result from a database.                                *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef _ECLLocalRunCalibLoader_H_
#define _ECLLocalRunCalibLoader_H_
// STL
#include <string>
#include <vector>
#include <utility>
// ROOT
#include <TTree.h>
namespace Belle2 {
  /**
   * The ECLLocalRunCalibLoader class
   * is designed to load tree of load tree
   * of the local run calibration results from
   * a database.
   */
  class ECLLocalRunCalibLoader {
  public:
    /**
     * Constructor.
     * @param isLocal enables usage of a local
     database.
     * @param dbName is a tag in the central database
     or path to a local database.
     * @param timeFilePath is path to a file with
     a run time table.
     */
    ECLLocalRunCalibLoader(bool isLocal,
                           const char* dbName,
                           const char* timeFilePath = "");
    /**
     * Destructor.
     */
    ~ECLLocalRunCalibLoader();
    /**
     * Get tree for given vector
     * of experiment and run numbers.
     * @param tree is the tree.
     * @param runs is the vector of the
     pairs (experiment number, run number).
     * @param withref enables loading of
     reference data.
     */
    void getTree(TTree** tree,
                 const std::vector <
                 std::pair<int, int> > & runs,
                 bool withref = false) const;
    /**
     * Get tree for given begin and end
     * times.
     * @param tree is the tree.
     * @param sqltime_begin is the begin time.
     * @param sqltime_end is the end time.
     * @param withref enables loading of
     reference data.
     */
    void getTree(TTree** tree,
                 const std::string& sqltime_begin,
                 const std::string& sqltime_end,
                 bool withref = false) const;
  private:
    /**
     * Get experiment and run number of
     * the corresponding reference mark
     * using experiment and run number
     * of the current calibration run.
     * @param exp is the experiment number
     of the current calibration run.
     * @param run is the run number of
     the current calibration run.
     * @param ref_exp is the experiment
     number of the corresponding reference
     run.
     * @param ref_run is the run number
     of the corresponding reference run.
     */
    void getReference(const int& exp,
                      const int& run,
                      int* ref_exp,
                      int* ref_run) const;
    /**
     * c_ncellids is the number of cell ids.
     */
    static const int c_ncellids;
    /**
     * Name of the time payload.
     */
    static const std::string c_timePayloadName;
    /**
     * Name of the amplitude payload.
     */
    static const std::string c_amplPayloadName;
    /**
     * Name of the reference mark payload.
     */
    static const std::string c_refPayloadName;
    /**
     * c_isLocal enables ussage of a local database.
     */
    const bool c_isLocal;
    /**
     * A tag in the central database or
     * path to a local database.
     */
    const std::string c_dbName;
    /**
     * Path to the file with run time table.
     */
    const std::string c_timeFilePath;
  };
}
#endif
