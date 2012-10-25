/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MDSTPIDTMPMODULE
#define MDSTPIDTMPMODULE

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /* a temporary module: example of how to fill PIDLikelihoods
   */

  class MdstPIDtmpModule : public Module {
  public:

    MdstPIDtmpModule();
    virtual ~MdstPIDtmpModule();
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();
    void printModuleParams() const;

  private:

  };

} // Belle2 namespace

#endif




