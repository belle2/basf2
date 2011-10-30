/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLRECCRMODULE_H_
#define ECLRECCRMODULE_H_

#include <framework/core/Module.h>
#include <vector>
#include <TRandom3.h>

namespace Belle2 {

  /** Module for making ECLRecoHits of CDCHits.
   *
   */
  class ECLRecCRModule : public Module {

  public:

    /** Constructor.
     */
    ECLRecCRModule();

    /** Destructor.
     */
    ~ECLRecCRModule();


    /** Initialize variables, print info, and start CPU clock. */
    virtual void initialize();

    /** Nothing so far.*/
    virtual void beginRun();

    /** Actual digitization of all hits in the ECL.
     *
     *  The digitized hits are written into the DataStore.
     */
    virtual void event();

    /** Nothing so far. */
    virtual void endRun();

    /** Stopping of CPU clock.*/
    virtual void terminate();



  protected:



  private:
    /** Name of output Hit collection of this module
     */
    std::string m_eclRecCRName;

    /**  Name of collection of ECLRecCR.
      */
    std::string m_eclDigiCollectionName;

    double m_timeCPU;                /*!< CPU time     */
    int    m_nRun;                   /*!< Run number   */
    int    m_nEvent;                 /*!< Event number */
    int    m_hitNum;




  };
}

#endif /* EVTMETAINFO_H_ */
