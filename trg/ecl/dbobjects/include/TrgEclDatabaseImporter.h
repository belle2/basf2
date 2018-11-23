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


#include <iostream>
#include <TObject.h>
#include <TVector3.h>


namespace Belle2 {
  class TrgEclDatabaseImporter;
  class TrgEclDatabaseImporter {

  public:

    /** Constructor */
    TrgEclDatabaseImporter();
    /** Destructor */
    virtual ~TrgEclDatabaseImporter() {}
    void setRunNumber(int start, int end)
    {
      startRun = start;
      endRun = end;
    }
    void setExpNumber(int start, int end)
    {
      startExp = start;
      endExp = end;
    }

  public:

    //-----------------------------------
    // FAM Parameters
    //------------------------------------
    /** Import FAM Parameters. */
    void importFAMParameter(std::string, std::string, std::string);
    //-----------------------------------
    // TMM Parameters
    //------------------------------------
    void importTMMParameter(std::string);

    //-----------------------------------
    // ETM Parameters
    //------------------------------------
    /** Import ETM Parameters */
    void importETMParameter(std::string);

    /**Print TC energy Threshold*/
    void printTCThreshold();
    //  double GetTCFLatency(int);
  private:
    int startExp;
    int startRun;
    int endExp;
    int endRun;

    ClassDef(TrgEclDatabaseImporter, 1);
  };
} // End namespace Belle2
#endif
