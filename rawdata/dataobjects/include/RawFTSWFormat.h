/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawFTSWFormat.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 7 - Mar - 2016
//-

#ifndef RAWFTSWFORMAT_H
#define RAWFTSWFORMAT_H
#include <rawdata/dataobjects/RawDataBlockFormat.h>

namespace Belle2 {

  /**
   * The Raw FTSW class.
   *
   * This class stores the RAW data containing FTSW data(event #, trg time ).
   */
  class RawFTSWFormat : public RawDataBlockFormat {
  public:

    //! Default constructor
    RawFTSWFormat();
    //! Constructor using existing pointer to raw data buffer
    //RawFTSWFormat(int* bufin, int nwords);
    //! Destructor
    virtual ~RawFTSWFormat();

    /*     //! Get # of words in this buffer */
    /*     int GetNwords(int n); */

    //! Get # of words of header
    virtual int GetNwordsHeader(int n) = 0;

    //! Get Node # ( should be "TTD " )
    virtual unsigned int GetFTSWNodeID(int n) = 0;

    //! Get event #
    virtual unsigned int GetEveNo(int n) = 0;

    //! Get a word containing ctime and trigger type info
    virtual unsigned int GetTTCtimeTRGType(int n) = 0;

    //! get unixtime of the trigger
    virtual unsigned int GetTTUtime(int n) = 0;

    //! Get ctime of the trigger
    virtual int GetTTCtime(int n) = 0;

    //! Get trgtype
    virtual int GetTRGType(int n) = 0;

    //! Get timeval from ctime and utime
    virtual void GetTTTimeVal(int n, struct timeval* tv) = 0;

    //! Get timespec from ctime and utime
    virtual void GetTTTimeSpec(int n, struct timespec* ts) = 0;

    //! Get time in ns since epoch from ctime and utime
    virtual unsigned long long int GetTTTimeNs(int n) = 0;

    //! Get magic number for data corruption check
    virtual unsigned int GetMagicTrailer(int n) = 0;

    //! Get stored information of getimeofday in a PC
    virtual void GetPCTimeVal(int n, struct timeval* tv);



    //! check the data contents
    virtual void CheckData(int n,
                           unsigned int prev_evenum, unsigned int* cur_evenum,
                           unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no) = 0;

    //! Exp# (10bit) run# (14bit) restart # (8bit)
    virtual unsigned int GetExpRunSubrun(int n) = 0;

    //! Get run #
    virtual int GetRunNo(int n) = 0;

    //! Get subrun #
    virtual int GetSubRunNo(int n) = 0;

    //! get a word cotaining run # and subrun #
    virtual int GetRunNoSubRunNo(int n) = 0;

    //! Get Exp #
    virtual int GetExpNo(int n) = 0;

    //! DESY test only (ver.1)
    virtual int Get15bitTLUTag(int n);

    //! HER injection = 1 or LER injection = 0
    virtual int GetIsHER(int n);

    //! Get time since the last injection
    virtual unsigned int GetTimeSinceLastInjection(int n);

    //! Get time since the previous trigger
    virtual unsigned int GetTimeSincePrevTrigger(int n);

    //! Get a bunch number
    virtual unsigned int GetBunchNumber(int n);

    //! Get a frame count
    virtual unsigned int GetFrameCount(int n);

  protected :

  };

}

#endif
