/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLRECCRMODULE_H_
#define ECLRECCRMODULE_H_

#include <framework/core/Module.h>
#include <vector>
#include <TRandom3.h>

namespace Belle2 {

  /**
   *
   * The detailed description of the ECLRecGamma module

    \correlationdiagram

    MCParticle = graph.external_data('MCParticle')
    CDCHit = graph.external_data('CDCHit')
    CDCTrackHit = graph.data('CDCTrackHit')
    CDCTrackCandidate = graph.data('CDCTrackCandidate')

    graph.module('CDCTracking', [MCParticle, CDCHit], [CDCTrackHit, CDCTrackCandidate])

    graph.relation(CDCTrackHit, CDCHit)
    graph.relation(CDCTrackCandidate, CDCTrackHit)
    graph.relation(CDCTrackCandidate, MCParticle)

    \endcorrelationdiagram

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

    /* function of sruare */
    double squ(double x) { return x * x; }

    /**calculate error of Energy with Energy  */
    double errorE(double E);

    /**calculate error of Theta with Energy and Theta  */
    double errorTheta(double Energy, double Theta);

    /**calculate error of Phi with Energy and Theta  */
    double errorPhi(double Energy, double Theta);


  protected:



  private:
    /** members of ECLRecCR Module
     */
    std::string m_eclMdstShowerName;  /** Name of collection of MdstShower */
    std::string m_eclDigiCollectionName;/** Name of collection of ECLDigi.*/
    std::string m_eclHitAssignmentName; /** Name of collection of ECLHitAssignment.*/
    double m_timeCPU;                /** CPU time     */
    int    m_nRun;                   /** Run number   */
    int    m_nEvent;                 /** Event number */
    int    m_hitNum;                 /** ECLRecCR index */
    int    m_HANum;                 /**  ECLHitAssignment. index */



  };
}

#endif /* EVTMETAINFO_H_ */
