/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDTRIGGERQUALITYGENERATORMODULE_H
#define SVDTRIGGERQUALITYGENERATORMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>

#include <string>
#include <cmath>
#include <cstdlib>

namespace Belle2 {
  /**This module generates a StoreObjPtr that contains random trigger quality chosen between FINE and CORSE
   */
  class SVDTriggerQualityGeneratorModule : public Module {

  public:
    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDTriggerQualityGeneratorModule();

    /** if required */
    virtual ~SVDTriggerQualityGeneratorModule();

    /** initializes the module */
    virtual void beginRun() override;

    /** processes the event */
    virtual void event() override;

    /** end the run */
    virtual void endRun() override;

    /** terminates the module */
    virtual void terminate() override;

    /** init the module */
    virtual void initialize() override;

  private:
    std::string m_trgQRGName = "TRGSummary";  /**< name of the output TRGQualityRG StoreObjPtr */
    StoreObjPtr<TRGSummary> m_triggerQRG;  /**<storage for TRGQualityRG object required for the module */
  };
}

#endif /* SVDTRIGGERQUALITYGENERATORMODULE_H */
