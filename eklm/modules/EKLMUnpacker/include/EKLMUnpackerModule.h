/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMUNPACKERMODULE_H
#define EKLMUNPACKERMODULE_H

/* C++ headers. */
#include <string>

/* Belle2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  //!Unpacker for the KLM
  class EKLMUnpackerModule : public Module {
  public:
    //! Constructor
    EKLMUnpackerModule();
    //! Destructor
    virtual ~EKLMUnpackerModule();
    //! Initialize at start of job
    virtual void initialize();
    //! begin run stuff
    virtual void beginRun();
    //! Unpack one event and create digits
    virtual void event();
    //! end run stuff
    virtual void endRun();
    //! Terminate at the end of job
    virtual void terminate();

  private:
    //! length of one hit in 4 byte words. This is needed find the hits in the detector buffer
    const int hitLength = 2;
    //! name of EKLMDigit store array
    std::string m_outputDigitsName;
  };

}
#endif
