/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclDataBase.h
// Section  : TRG ECL
// Owner    : InSoo Lee / Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------
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
      startRun = start;
      endRun = end;
    }
    //! Set Experimnet Number
    void setExpNumber(int start, int end)
    {
      startExp = start;
      endExp = end;
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
    int startExp;
    //! Start Run Number
    int startRun;
    //! End Experiment Number
    int endExp;
    //! End Run Number
    int endRun;
  };
} // End namespace Belle2
#endif
