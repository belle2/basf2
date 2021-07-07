#pragma once
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/core/Module.h>

namespace Belle2 {
  /** The Prescale module.
   *
   * This module randomly returns true or false on each event with a fraction determined by an input parameter.
   *
   * Can be used with a conditional basf2 path to control the number of events passing this module.
   */
  class PrescaleModule : public Module {
  public:
    /** Constructor of the module. Sets the description of the module and the parameters of the module. */
    PrescaleModule();

    /** Destructor of the module. */
    virtual ~PrescaleModule() {};

    /** No Initialization necessary */
    virtual void initialize() override {};

    /** Sets the return value by running a RNG vs. the prescale value */
    virtual void event() override;

  private:
    /** Prescale module parameter, this fraction of events will return True. [0.0 -> 1.0] */
    double m_prescale;
  };
}
