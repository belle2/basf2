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

  /**
   * EKLM unpacker.
   */
  class EKLMUnpackerModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMUnpackerModule();

    /**
     * Destructor.
     */
    virtual ~EKLMUnpackerModule();

    /**
     * Initializer.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     */
    virtual void beginRun();

    /**
     * This method is called for each event.
     */
    virtual void event();

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun();

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate();

  private:

    /**
     * Length of one hit in 4 byte words. This is needed find the hits in the
     * detector buffer
     */
    const int hitLength = 2;

    /** Name of EKLMDigit store array. */
    std::string m_outputDigitsName;

  };

}

#endif

