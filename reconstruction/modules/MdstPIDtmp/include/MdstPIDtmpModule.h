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

  /**
   * a temporary module: example of how to fill PIDLikelihoods
   * maybe we keep it, but with different name
   */

  class MdstPIDtmpModule : public Module {
  public:


    /**
     * Constructor
     */
    MdstPIDtmpModule();

    /**
     * Destructor
     */
    virtual ~MdstPIDtmpModule();

    /**
     * Initialize the module.
     * data store registration of PIDLikelihoods and relations to GFTracks
     */
    virtual void initialize();

    /**
     * Called when a new run is started.
     * currently empty
     */
    virtual void beginRun();

    /**
     * Called for each event.
     * loop over GFTracks, collect likelihoods and fill PIDLikelihoods
     */
    virtual void event();

    /**
     * Called when run ended.
     * currently empty
     */
    virtual void endRun();

    /**
     * Terminates the module.
     * currently empty
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     * currently empty
     */
    void printModuleParams() const;

  private:

  };

} // Belle2 namespace

#endif




