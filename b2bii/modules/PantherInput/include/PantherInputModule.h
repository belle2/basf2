//+
// File : PantherInputModule.h
// Description : A module to read Panther record in basf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//-

#ifndef PANTHER_INPUT_H
#define PANTHER_INPUT_H

#include <framework/core/Module.h>

#define BELLE_TARGET_H "belle-x86_64-unknown-linux-gnu-g++.h"
#include "panther/panther.h"
#include "panther/panther_group.h"

#include <string>

#include <sys/time.h>

namespace Belle2 {
  /** A class definition of an input module for Sequential ROOT I/O. */

  class PantherInputModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    PantherInputModule();
    virtual ~PantherInputModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Panther -> basf2 conversion function
    void Convert();


    // Data members
  private:
    //! File name
    std::string m_inputFileName;

    //! PantherFile
    Belle::Panther_FileIO* m_fd;

    //! Event counter
    int m_nevt;

  };

} // end namespace Belle2

#endif
