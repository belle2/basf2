/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLDATABASEIMPORTER_H
#define TRGECLDATABASEIMPORTER_H

#include <Rtypes.h>

#include <string>

namespace Belle2 {
  /// Database importer of TRGECL
  class TrgEclDatabaseImporter {

  public:

    //! TrgEclDatabaseImporter Constructor
    TrgEclDatabaseImporter();
    //! TrgEclDatabaseImporter Destructor
    virtual ~TrgEclDatabaseImporter() {}
    //! Set Run Number
    void setRunNumber(int start, int end)
    {
      m_startRun = start;
      m_endRun = end;
    }
    //! Set Experiment Number
    void setExpNumber(int start, int end)
    {
      m_startExp = start;
      m_endExp = end;
    }

  public:

    //! Import FAM Parameters
    void importFAMParameter(std::string, std::string, std::string);
    //! Import TMM Parameters
    void importTMMParameter(std::string);
    //! Import ETM Parameters
    void importETMParameter(std::string);
    //! Import Bad Run Number
    void importBadRunNumber(std::string);

    //!Print TC energy Threshold
    void printTCThreshold();

  private:
    //! Start Experiment Number
    int m_startExp;
    //! Start Run Number
    int m_startRun;
    //! End Experiment Number
    int m_endExp;
    //! End Run Number
    int m_endRun;
  };
} // End namespace Belle2
#endif
