/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DESERIALIZERCOPPER_H
#define DESERIALIZERCOPPER_H

#include <fcntl.h>
#include <errno.h>

#include <framework/datastore/StoreArray.h>

#include <daq/rawdata/modules/DeSerializer.h>

#include <rawdata/dataobjects/PreRawCOPPERFormat_latest.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */



  class DeSerializerCOPPERModule : public  DeSerializerModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerCOPPERModule();
    virtual ~DeSerializerCOPPERModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();

    /* #ifndef REDUCED_RAWCOPPER */
    /*     //! Fill RawHeader */
    /*     virtual void fillNewRawCOPPERHeader(RawCOPPER* raw_copper); */
    /* #endif */



#ifdef NONSTOP
    //! for nonstop DAQ
    virtual void resumeRun();

    //! for nonstop DAQ
    virtual void waitResume();
#endif

  protected:
    //! Handle to data from HSLBs
    const PreRawCOPPERFormat_latest m_pre_rawcpr;

  private:


    StoreArray<RawDataBlock> raw_dblkarray;

    virtual int* readOneEventFromCOPPERFIFO(const int entry, int* delete_flag, int* m_size_word);
    virtual void openCOPPER();
    //! receive data
    virtual int readFD(int fd, char* buf, int data_size_byte, int delete_flag);

    //! Module functions to be called from main process
    virtual void initializeCOPPER();

    // Parallel processing parameters

    //! COPPER file descripter
    int m_cpr_fd;

    //! bit flag for available FINESSE slot
    int m_use_slot;

    //! bit flag for available FINESSE slot
    int m_finesse_bit_flag;



    //! Event # from data
    unsigned int m_prev_ftsweve32;

    //! exp #
    int m_exp;

    //! run #
    int m_run;


  };

} // end namespace Belle2

#endif
