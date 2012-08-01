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
  namespace ECL {
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
    class ECLRecShowerModule : public Module {

    public:

      /** Constructor.
       */
      ECLRecShowerModule();

      /** Destructor.
       */
      ~ECLRecShowerModule();


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

      /** function of sruare */
      double squ(double x) { return x * x; }

      /**calculate error of Energy with Energy  */
      float errorE(double E);

      /**calculate error of Theta with Energy and Theta  */
      float errorTheta(double Energy, double Theta);

      /**calculate error of Phi with Energy and Theta  */
      float errorPhi(double Energy, double Theta);


    protected:



    private:
      /** members of ECLRecShower Module
       */
      /** Name of collection of ECLShower */
      std::string m_ECLShowerName;
      /** Name of collection of ECLDigi.*/
      std::string m_eclDigiCollectionName;
      /** Name of collection of ECLHitAssignment.*/
      std::string m_eclHitAssignmentName;
      /** CPU time     */
      double m_timeCPU;
      /** Run number   */
      int    m_nRun;
      /** Event number */
      int    m_nEvent;
      /** ECLRecShower index */
      int    m_hitNum;
      /**  ECLHitAssignment. index */
      int    m_HANum;



    };
  }//ECL
}//Belle2

#endif /* EVTMETAINFO_H_ */
