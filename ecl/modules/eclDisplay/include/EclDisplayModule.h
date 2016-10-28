/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Remnev Mikhail                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDISPLAYMODULE_H
#define ECLDISPLAYMODULE_H

#include <rawdata/dataobjects/RawECL.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <cstdio>
#include <TROOT.h>
#include <TRint.h>
#include <TRootEmbeddedCanvas.h>
#include <unistd.h>
#include <fcntl.h>

#include <framework/core/Module.h>
#include <ecl/modules/eclDisplay/EclFrame.h>
#include <TThread.h>

namespace Belle2 {
  /**
   * Displays energy distribution in ECL.
   *
   * This module implements multiple presentations of RawECL data in the
   * user-defined interval.
   *
   */
  class EclDisplayModule : public Module {

  public:

    /**  */
    EclDisplayModule();

    /**  */
    virtual ~EclDisplayModule();

    void startThread(int in);

    /**  */
    virtual void initialize();

    /**  */
    virtual void beginRun();

    /**  */
    virtual void event();

    /**  */
    virtual void terminate();

    virtual void serializeAndSend(int ch, int amp, int time, int evtn);

  private:
    void ReadEclEvent(RawCOPPER* raw_copper, int evt_num);
    void AddEclCluster(StoreArray<RawECL>& raw_eclarray);

    bool m_displayEnergy;  /**< If true, energy distribution in ECL is displayed. Frequency of events per crystal is displayed otherwise. */
    int m_displayMode;  /**< Default display mode. Can be later changed in GUI. */
    bool m_autoDisplay;  /**< If true, events are displayed as soon as they are loaded. */

    int m_out;
  };
}

#endif /* ECLDISPLAYMODULE_H */
