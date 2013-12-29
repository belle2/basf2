//+
// File : ErrorMessage.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef ERRORMESSAGE_H
#define ERRORMESSAGE_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <TObject.h>
#include <string>
//#include <daq/slc/readout/ProcessStatusBuffer.h>

namespace Belle2 {

  class ErrorMessage : public TObject {
  public:
    //! Default constructor
    ErrorMessage();

    //! Default constructor
    //    ErrorMessage( const char* entry_name );


    //! Destructor
    virtual ~ErrorMessage();

    //!
    void PrintError(char* err_message, const char* file, const char* func_name, const int line);

    //!
    //    void PrintError(const int shmflag, ProcessStatusBuffer* nsm_status, char* err_message, const char* file, const char* func_name, const int line);
    //!
    //    void PrintError( ProcessStatusBuffer* nsm_status, char* err_message, const char* file, const char* func_name, const int line);
    //    void PrintError(const std::string err_message, const char* file, const char* func_name, const int line);

    ClassDef(ErrorMessage, 1);
  };
}

#endif
