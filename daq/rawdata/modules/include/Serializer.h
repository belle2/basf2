/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <string>

#include <framework/core/Module.h>

#include <daq/dataflow/EvtSocket.h>

#include <daq/rawdata/CprErrorMessage.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>
#include <rawdata/dataobjects/RawDataBlock.h>

#include <rawdata/dataobjects/RawHeader_latest.h>

#include <sys/mman.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class SerializerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    SerializerModule();
    virtual ~SerializerModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    virtual int sendByWriteV(RawDataBlock* rawdblk);
    //    virtual void Connect(const char* hostname, const int port);
    virtual void Accept();

    virtual void fillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl, RawDataBlock* rawdblk);
    // Data members

    //! calculate checksum
    unsigned int calcXORChecksum(int* buf, int nwords);

    //! Use shared memory
    int m_shmflag;

    //! open shared memory
    void shmOpen(char* path_cfg, char* path_sta);

    //! Get shared memory
    int* shmGet(int fd, int size_words);

    //! send buffer
    int Send(int socket, char* buf, int size_bytes);

    //! print data contents
    void printData(int* buf, int nwords);

    //! Check socket status
    int CheckConnection(int socket);

    //! file descripter for shm
    int m_shmfd_cfg;

    //! file descripter for shm
    int m_shmfd_sta;

    //! buffer for shared memory
    int* m_cfg_buf;

    //! buffer for shared memory
    int* m_cfg_sta;


  protected :

    enum {
      COPPER = 1,
      ROPC = 2
    };

    //! size of buffer for one event (word)
    int BUF_SIZE_WORD;

    //!Compression parameter
    int m_compressionLevel;

    //! No. of sent events
    int n_basf2evt;

    int* m_buffer;

    // Parameters for EvtSocket

    //! Destination Host
    std::string m_hostname_local;

    //! Destination port
    int m_port_to;

    //! Socket ID
    int m_socket;

    //! EvtSocket
    EvtSocketSend* m_sock;

    double m_start_time;

    double m_prev_time;

    unsigned int m_prev_nevt;

    double m_prev_totbytes;

    double time_array0[1000];
    double time_array1[1000];
    double time_array2[1000];

    //! How to handle data
    std::string p_method;
    int p_method_val;

    //! store time info.
    double getTimeSec();

    //! store time info.
    void recordTime(int event, double* array);

    //! error message program
    CprErrorMessage print_err;

    //! sent data size
    unsigned long long m_totbytes;

    //! start flag
    int m_start_flag;

#ifdef NONSTOP
    //! for nonstop DAQ
    int* m_ptr;

    //! for nonstop DAQ
    virtual void openRunPauseNshm();

    //! for nonstop DAQ
    virtual int checkRunPause();

    //! for nonstop DAQ
    virtual void resumeRun();

    //! check RunPause
    void callCheckRunPause(std::string& err_str);

#endif

    //! which format is used
    RawHeader_latest tmp_header;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
