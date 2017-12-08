/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCDEDXSCANMODULE_H
#define CDCDEDXSCANMODULE_H

#include <reconstruction/dataobjects/DedxConstants.h>

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>

namespace Belle2 {

  class CDCDedxTrack;

  /** This class performs the same function as CDCDedxPIDModule, but does so
   * without using real objects from BASF2. Instead, it scans values of
   * DOCA and entrance angle for a cell in each layer of the CDC.
   */
  class CDCDedxScanModule : public Module {

  public:

    /** Default constructor */
    CDCDedxScanModule();

    /** Destructor */
    virtual ~CDCDedxScanModule();

    /** Initialize the module */
    virtual void initialize();

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event();

    /** End of the event processing. */
    virtual void terminate();

  private:

    // register outputs
    StoreArray<CDCDedxTrack> m_dedxArray; /**< array of output dE/dx tracks */

  };
} // Belle2 namespace
#endif
