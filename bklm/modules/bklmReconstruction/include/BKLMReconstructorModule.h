#ifndef BKLMRECONSTRUCTIONMODULE_H
#define BKLMRECONSTRUCTIONMODULE_H

#include <framework/core/Module.h>

namespace Belle2 {

  namespace bklm {
    class GeometryPar;
  }

  //! Module BKLMReconstructorModule.
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
    virtual void initialize();

    //! Prepare for start of each run
    virtual void beginRun();

    //! Process each event
    virtual void event();

    //! Aggregate information at end of each run
    virtual void endRun();

    //! Once-only termination at the end of the job
    virtual void terminate();

  protected:

  private:

    //! Pointer to GeometryPar singleton
    bklm::GeometryPar* m_GeoPar;

    //! maximum time difference (ns) between orthogonal strips that are in coincidence
    double m_DtMax;

    //! mean prompt time (ns) of 2D hit
    double m_PromptTime;

    //! half-width window (ns) of 2D hit relative to mean prompt time
    double m_PromptWindow;

  };
} // end namespace Belle2
#endif

