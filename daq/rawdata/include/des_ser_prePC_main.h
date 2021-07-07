/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DES_SER_PREPC_MAIN_H
#define DES_SER_PREPC_MAIN_H

#include <string>
#include <vector>
#include <stdlib.h>
/* #include <sys/time.h> */

/* #include <framework/core/Module.h> */
/* #include <framework/pcore/EvtMessage.h> */
/* #include <framework/pcore/MsgHandler.h> */
/* #include <framework/datastore/DataStore.h> */
/* #include <framework/datastore/StoreObjPtr.h> */
/* #include <framework/datastore/StoreArray.h> */
/* #include <framework/dataobjects/EventMetaData.h> */

/* #include <daq/dataflow/EvtSocket.h> */
/* #include <daq/slc/readout/RunInfoBuffer.h> */
/* #include <daq/rawdata/CprErrorMessage.h> */
/* #include <daq/rawdata/modules/DAQConsts.h> */


/* #include <rawdata/dataobjects/RawDataBlock.h> */
//#include <rawdata/dataobjects/RawCOPPER.h>

/* #include <sys/socket.h> */
/* #include <sys/types.h> */
/* #include <arpa/inet.h> */
/* #include <netinet/in.h> */
/* #include <netinet/tcp.h> */

/* #include <sys/mman.h> */
/* #include <sys/stat.h> */
/* #include <fcntl.h> */

/* namespace Belle2 { */

/*   /\*! A class definition of an input module for Sequential ROOT I/O *\/ */

/*   class des_ser_prePC_main{ */

/*     // Public functions */
/*   public: */

/*     //! Constructor / Destructor */
/*     des_ser_prePC_mainModule(); */
/*     virtual ~des_ser_prePC_mainModule(); */

/*     //! Module functions to be called from main process */
/*     virtual void initialize(); */

/*     //! Module functions to be called from main process */
/*     virtual void event(); */

/*   protected : */
/*     //! hostname */
/*     string m_host_recv; */

/*     //! hostname */
/*     string m_host_send; */

/*     //! port number */
/*     int m_port_recv; */

/*     //! port number */
/*     int m_port_send; */

/*     //! Messaage handler */
/*     MsgHandler* m_msghandler; */

/*     //! Compression Level */
/*     int m_compressionLevel; */

/*     //! Node(PC or COPPER) ID */
/*     int m_nodeid; */

/*     //! Node name */
/*     std::string m_nodename; */

/*     //! Use shared memory -> 1; Without shm -> 0 */
/*     int m_shmflag; */

/*     // Event Meta Data */
/*     StoreObjPtr<EventMetaData> m_eventMetaDataPtr; */

/*   private: */


/*   public: */

/*   }; */

/* } // end namespace Belle2 */

#endif // MODULEHELLO_H
