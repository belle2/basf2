/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2019 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Marco Milesi (marco.milesi@unimelb.edu.au)               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /** A module template.
   *
   *  A detailed description of your module.
   */
  class ChargedPidMVAModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    ChargedPidMVAModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~ChargedPidMVAModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, StoreObjPtrs, relations) here,
     *  see the respective class documentation for details.
     */
    virtual void initialize() override;

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun() override;

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event() override;

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun() override;

    /** Clean up anything you created in initialize(). */
    virtual void terminate() override;
  private:
    //define your own data members here

  protected:
    //definition of input parameters

  };
}

