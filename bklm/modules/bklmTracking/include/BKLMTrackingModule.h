/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef  BKLMTRACKING_H
#define  BKLMTRACKING_H

#include <framework/core/Module.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/dataobjects/BKLMTrack.h>
#include <bklm/modules/bklmTracking/BKLMTrackFinder.h>
#include <bklm/geometry/Module.h>

namespace Belle2 {

  class BKLMTrackingModule: public Module {

  public:

    //! Constructor
    BKLMTrackingModule();

    //! Destructor
    virtual ~BKLMTrackingModule();

    //! Initialize at start of job
    virtual void initialize();

    //! begin run stuff
    virtual void beginRun();

    //! Unpack one event and create digits
    virtual void event();

    //! end run stuff
    virtual void endRun();

    //! Terminate at the end of job
    virtual void terminate();

    //! Judge if two hits come from the same sector
    bool sameSector(BKLMHit2d* hit1, BKLMHit2d* hit2);

  protected:

    //! Mean hit - trigger time (ns)
    double m_MeanDt;

    //! Coincidence window half-width for in-time KLM hits (ns)
    double m_MaxDt;

  private:


  };
} // end namespace Belle2
#endif
