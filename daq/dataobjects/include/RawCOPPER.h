//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWCOPPER_H
#define RAWCOPPER_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <daq/dataobjects/RawDataBlock.h>
#include <daq/dataobjects/RawHeader.h>
#include <daq/dataobjects/RawTrailer.h>
#include <framework/datastore/DataStore.h>

#include <daq/rawdata/modules/DAQConsts.h>

#include <TObject.h>

namespace Belle2 {


  class RawCOPPER : public RawDataBlock {
  public:
    //! Default constructor
    RawCOPPER();
    //! Constructor using existing pointer to raw data buffer
    RawCOPPER(int* bufin, int nwords);
    //! Destructor
    virtual ~RawCOPPER();

    //
    // Get position of or pointer to data
    //
    ///////////////////////////////////////////////////////////////////////////////////////
    // POINTER TO "DETECTOR BUFFER"
    //( after removing "B2link headers" from "FINNESSE buffer". THIS IS THE RAW DATA FROM A DETECTOR
    ///////////////////////////////////////////////////////////////////////////////////////
    //! get Detector buffer length of slot A
    int Get1stDetectorNwords(int n);

    //! get Detector buffer length of slot B
    int Get2ndDetectorNwords(int n);

    //! get Detector buffer length of slot C
    int Get3rdDetectorNwords(int n);

    //! get Detector buffer length of slot D
    int Get4thDetectorNwords(int n);

    //! get Detector buffer of slot A
    int* Get1stDetectorBuffer(int n);

    //! get Detector Buffer of slot B
    int* Get2ndDetectorBuffer(int n);

    //! get Detector Buffer of slot C
    int* Get3rdDetectorBuffer(int n);

    //! get Detector Buffer of slot D
    int* Get4thDetectorBuffer(int n);
    ///////////////////////////////////////////////////////////////////////////////////////

    //! get posistion of COPPER block in unit of word
    virtual int GetBufferPos(int n);

    //! get buffer pointer of rawcopper header(Currently same as GetBufferPos)
    virtual int* GetRawHdrBufPtr(int n);

    //! get buffer pointer of rawcopper trailer
    virtual int* GetRawTrlBufPtr(int n);

    //! get FINNESSE buffer pointer for slot A
    int* Get1stFINNESSEBuffer(int n);

    //! get FINNESSE buffer pointer for slot B
    int* Get2ndFINNESSEBuffer(int n);

    //! get FINNESSE buffer pointer for slot C
    int* Get3rdFINNESSEBuffer(int n);

    //! get FINNESSE buffer pointer for slot D
    int* Get4thFINNESSEBuffer(int n);


    //
    // Get information from "RawCOPPER header" attached by DAQ software
    //
    int GetExpNo(int n);    //! get contents of header

    int GetRunNo(int n);    //! get contents of header

    unsigned int GetEveNo(int n);    //! get contents of header

    int GetDataType(int n);    //! get contents of header

    int GetTruncMask(int n);    //! get contents of header

    int GetSubsysId(int n);     //! get subsystem-ID from data

    //
    // Get information from 13words "COPPER header" attached by COPPER board
    //
    //! get COPPER counter(not event number)
    virtual unsigned int GetCOPPERCounter(int n);

    //! get COPPER node id from data(Currently same as GetCOPPERNodeId)
    virtual int GetCOPPERNodeId(int n);

    //! get # of FINNESEs which contains data
    virtual int GetNumFINNESSEBlock(int n);

    //! get # of offset words for FINNESSE slot A buffer position
    int GetOffset1stFINNESSE(int n);

    //! get # of offset words for FINNESSE slot B buffer position
    int GetOffset2ndFINNESSE(int n);

    //! get # of offset words for FINNESSE slot C buffer position
    int GetOffset3rdFINNESSE(int n);

    //! get # of offset words for FINNESSE slot D buffer position
    int GetOffset4thFINNESSE(int n);

    //! get data size of  FINNESSE slot A buffer
    int Get1stFINNESSENwords(int n);

    //! get data size of  FINNESSE slot B buffer
    int Get2ndFINNESSENwords(int n);

    //! get data size of  FINNESSE slot C buffer
    int Get3rdFINNESSENwords(int n);

    //! get data size of  FINNESSE slot D buffer
    int Get4thFINNESSENwords(int n);

    //
    // Get information from "B2link(attached by FEE and HLSB) header"
    //
    //! get b2l block from "FEE b2link header"
    virtual int* GetFTSW2Words(int n);

    //! get b2l block from "FEE b2link header"
    virtual int* GetExpRunBuf(int n);

    //! get b2l block from "FEE b2link header"
    virtual int GetFTSW16bitEventNumber(int n);

    //! get Event unixtime from "FEE b2link header"
    double GetEventUnixTime(int n);

    //
    // read magic word to check data
    //
    //! get magic word for the beginning of  COPPER data block
    unsigned int GetMagic7FFF0008(int n);

    //! get magic word in "COPPER header"
    unsigned int GetMagicFFFFFAFA(int n);

    //! get magic word for the end of  COPPER data block
    unsigned int GetMagic7FFF0009(int n);


    //
    // size of "COPPER front header" and "COPPER trailer"
    //
    enum {
      SIZE_COPPER_FRONT_HEADER = 7,
      SIZE_COPPER_TRAILER = 3
    };

    //
    // Data Format : "COPPER header"
    //
    enum {
      POS_MAGIC_COPPER_1 = 0,
      POS_EVE_NUM_COPPER = 1,
      POS_SUBSYSTEM_ID = 2,
      POS_CRATE_ID = 3,
      POS_SLOT_ID = 4,
      POS_MAGIC_COPPER_2 = 7,
      POS_DATA_LENGTH = 8,
      POS_CH_A_DATA_LENGTH = 9,
      POS_CH_B_DATA_LENGTH = 10,
      POS_CH_C_DATA_LENGTH = 11,
      POS_CH_D_DATA_LENGTH = 12,

      SIZE_COPPER_HEADER = 13
    };

    //
    // Data Format : "COPPER Trailer"
    //
    enum {
      POS_MAGIC_COPPER_3 = 0,
      POS_CHKSUM_COPPER = 1,
      POS_MAGIC_COPPER_4 = 2

    };

    //
    // Data Format : "B2Link HSLB Header"
    //
    enum {
      POS_MAGIC_B2LHSLB = 0,
      //      POS_EVE_CNT_B2LHSLB = 1,
      SIZE_B2LHSLB_HEADER = 1
    };

    //
    // Data Format : "B2Link HSLB Trailer"
    //
    enum {
      POS_CHKSUM_B2LHSLB = 0,
      SIZE_B2LHSLB_TRAILER = 1
    };

    //
    // Data Format : "B2Link FEE Header"
    //
    enum {
      POS_FTSW1 = 0,
      POS_FTSW2 = 1,
      POS_EXP_RUN = 2,
      POS_B2L_TIME = 3,

      SIZE_B2LFEE_HEADER = 4 // modified by Oct.3, 2013, Nakao-san's New firmware?
                           //      SIZE_B2LFEE_HEADER = 1  // As of Aug.22,2013, the header size is one word. It should be 2 in future.
    };

    //
    // Data Format : B2Link FEE Trailer
    //
    enum {
      POS_CHKSUM_B2LFEE = 0,

      SIZE_B2LFEE_TRAILER = 1
    };



  protected :

    RawHeader tmp_header;  //! Not record

    RawTrailer tmp_trailer; //! Not record

    ClassDef(RawCOPPER, 1);
  };
}

#endif
