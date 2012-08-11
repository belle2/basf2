/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLGAMMARECONSTRUCTORMODULE_H_
#define ECLGAMMARECONSTRUCTORMODULE_H_

#include <framework/core/Module.h>
#include <vector>
#include <TRandom3.h>

namespace Belle2 {
  namespace ECL {

    /**
     * ECL Reconstruction Gamma Module.
     *
     * The detailed description of the ECLGammaReconstructorModule

      \correlationdiagram

      GFTrackCand = graph.external_data('GFTrackCand')
      ECLGammaReconstructor = graph.external_data('ECLGammaReconstructor')
      ECLHitAssignment = graph.data('ECLHitAssignment')
      ECLShower = graph.data('ECLShower')
      ECLGamma = graph.data('ECLGamma')
      graph.module('ECLGammaReconstructor', [GFTrackCand, ECLShower], [ECLGamma])

      \endcorrelationdiagram

     */
    class ECLGammaReconstructorModule : public Module {

    public:

      /** Constructor.
       */
      ECLGammaReconstructorModule();

      /** Destructor.
       */
      ~ECLGammaReconstructorModule();


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

      /** goodGamma selector */
      bool goodGamma(double ftheta, double energy, double nhit, double fe9oe25, double fwidth, double ecut = 0.02, double e925cut = 0.75, double widcut = 6.0, double nhcut = 0);

      /** read Extrapolate CellID */
      void readExtrapolate();
    protected:



    private:
      /** members of ECLGammaReconstructor Module
       */
      /** Name of input collection of this module */
      std::string m_ECLShowerName;
      /**  Name of collection of ECLHitAssignment.*/
      std::string m_eclHitAssignmentName;
      /**  Name of collection of ECLGamma.*/
      std::string m_ECLGammaName ;
      /** Name of the GFTrack collection of the reconstructed tracks to be extrapolated */
      std::string m_gfTracksColName;

      /** Name of the GFTrackCand collection, each holding the list of hits for a particular track and hypothesis */
      std::string m_extTrackCandsColName;

      /** Name of the extRecoHit collection of the extrapolation hits */
      std::string m_extRecoHitsColName;

      /** CPU time     */
      double m_timeCPU;
      /** Run number   */
      int    m_nRun;
      /** Event number */
      int    m_nEvent;
      /** ECL Gamma index */
      int    m_GNum;
      /** extrapolated cell */
      bool   m_TrackCellId[8736];
      /** extrapoltion match */
      bool   m_extMatch;
      /** shower information    */
      int    m_showerId ;
      /** shower information    */
      double m_energy;
      /** shower information    */
      double m_Theta;
      /** shower information    */
      double m_theta;
      /** shower information    */
      double m_phi ;
      /** shower information    */
      double m_e9oe25 ;
      /** shower information    */
      double m_width  ;
      /** shower information    */
      double m_nhit   ;
      /** shower information    */
      int    m_quality;


      /** ECLHitAssignment information    */
      int    m_HAshowerId ;
      /** ECLHitAssignment information    */
      int    m_HAcellId;

      /** good Gammacut m_ecut=0.02        */
      double m_ecut;
      /** good Gammacut m_e925cut=0.75        */
      double m_e925cut;
      /** good Gammacut m_widcut=6.0        */
      double m_widcut;
      /** good Gammacut m_nhcut=0        */
      double m_nhcut;

    };
  }//ECL
}//belle2

#endif
