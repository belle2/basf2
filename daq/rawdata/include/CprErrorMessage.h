//+
// File : CprErrorMessage.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 29 - Dec - 2013
//-

#ifndef CPRERRORMESSAGE_H
#define CPRERRORMESSAGE_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string>
//#include <framework/core/Module.h>
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
    void PrintError(char* err_message, const char* file, const char* func_name, const int line);

    //!
    void PrintError(const int shmflag, RunInfoBuffer* nsm_status, char* err_message, const char* file, const char* func_name, const int line);

    //!
    void PrintError(const int shmflag, RunInfoBuffer* nsm_status, std::string err_str);

    //!
    void PrintError(const char* err_message);


  };
}

#endif
