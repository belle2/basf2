/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DEDXSCANMODULE_H
#define DEDXSCANMODULE_H

#include <reconstruction/modules/DedxCorrection/DedxCorrectionModule.h>
#include <reconstruction/dataobjects/DedxConstants.h>

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>

using namespace std;

namespace Belle2 {

  class DedxCell;

  /** This class performs the same function as DedxCellModule, but does so
   * without using real objects from BASF2. Instead, it scans values of
   * DOCA and entrance angle for a cell in each layer of the CDC.
   */
  class DedxScanModule : public Module {

  public:

    /** Default constructor */
    DedxScanModule();

    /** Destructor */
    virtual ~DedxScanModule();

    /** Initialize the module */
    virtual void initialize();

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event();

    /** End of the event processing. */
    virtual void terminate();

  private:

    int m_trackID; /** counter for tracks */
    int m_eventID; /** counter for events */

  };
} // Belle2 namespace
#endif
