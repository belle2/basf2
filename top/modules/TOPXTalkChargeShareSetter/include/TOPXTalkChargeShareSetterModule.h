/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rikuya Okuto                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * Crosstalk & chargeshare flag setter
   *
   *
   */
  class TOPXTalkChargeShareSetterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TOPXTalkChargeShareSetterModule();

    /** destructor */
    virtual ~TOPXTalkChargeShareSetterModule() override;

    /** initialize */
    virtual void initialize() override;

    /** beginRun */
    virtual void beginRun() override;

    /** event */
    virtual void event() override;

    /** endRun */
    virtual void endRun() override;

    /** terminate */
    virtual void terminate() override;


  private:

  };
}
