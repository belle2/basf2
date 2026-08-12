/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CPRERRORMESSAGE_H
#define CPRERRORMESSAGE_H

// Includes
#include <string>
#include <daq/slc/readout/RunInfoBuffer.h>

namespace Belle2 {

  class CprErrorMessage {
  public:
    //! Default constructor
    CprErrorMessage();

    //! Default constructor
    //    CprErrorMessage( const char* entry_name );


    //! Destructor
    virtual ~CprErrorMessage();

    //!
    static void PrintError(const char* err_message, const char* file, const char* func_name, int line);

    //!
    static void PrintError(const int shmflag, RunInfoBuffer* nsm_status, const char* err_message, const char* file,
                           const char* func_name,
                           const int line);

    //!
    static void PrintError(int shmflag, Belle2::RunInfoBuffer* nsm_status, const std::string& err_str);

    //!
    static void PrintError(const char* err_message);


  };
}

#endif
