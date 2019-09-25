
#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <klm/bklm/dbobjects/BKLMTimeWindow.h>
#include <klm/bklm/dataobjects/BKLMDigit.h>
#include <klm/bklm/dataobjects/BKLMHit1d.h>
#include <klm/bklm/dataobjects/BKLMHit2d.h>

namespace Belle2 {

  namespace bklm {
    class GeometryPar;
  }

  /*!
    Create BKLMHit1ds from BKLMDigits then create BKLMHit2ds from BKLMHit1ds.
    Store relations among these.
  */
  class BKLMReconstructorModule : public Module {

  public:

    //! Constructor
    BKLMReconstructorModule();

    //! Destructor
    virtual ~BKLMReconstructorModule();

    //! Once-only initialization at start of job
    virtual void initialize() override;

    //! Prepare for start of each run
    virtual void beginRun() override;

    //! Process each event
    virtual void event() override;

    //! Aggregate information at end of each run
    virtual void endRun() override;

    //! Once-only termination at the end of the job
    virtual void terminate() override;

  private:

    //! Pointer to GeometryPar singleton
    bklm::GeometryPar* m_GeoPar;

    //! maximum time difference (ns) between orthogonal strips that are in coincidence
    double m_DtMax;

    //! mean prompt time (ns) of 2D hit
    double m_PromptTime;

    //! half-width window (ns) of 2D hit relative to mean prompt time
    double m_PromptWindow;

    //! flag for alignment correction, do the correction (true) or not (false)
    bool m_ifAlign;

    //! load timing cut from database or not
    bool m_loadTimingFromDB;

    //! time window parameters from dbobject
    DBObjPtr<BKLMTimeWindow> m_timing;

    //! digits StoreArray
    StoreArray<BKLMDigit> digits;

    //! hit1ds StoreArray
    StoreArray<BKLMHit1d> hit1ds;

    //! hit2ds StoreArray
    StoreArray<BKLMHit2d> hit2ds;

  };
} // end namespace Belle2
