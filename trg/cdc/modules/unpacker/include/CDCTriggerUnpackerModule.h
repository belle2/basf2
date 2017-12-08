/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tzu-An Sheng                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCTRIGGERUNPACKERMODULE_H
#define CDCTRIGGERUNPACKERMODULE_H

#include <framework/core/Module.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <framework/datastore/StoreArray.h>


namespace Belle2 {
  /**
   * Unpack the data recorded in B2L
   *
   * to be added
   *
   */
  class CDCTriggerUnpackerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CDCTriggerUnpackerModule();

    /** Register input and output data */
    virtual void initialize();

    /** convert raw data (in B2L buffer to bitstream) */
    virtual void event();


  private:

    bool m_merger;  /**< flag to unpack merger data (recorded by Merger Reader / TSF) */
    StoreArray<RawTRG> m_rawTrigger; /**< array containing the raw trigger data object */
  };
}

#endif /* CDCTRIGGERUNPACKERMODULE_H */
