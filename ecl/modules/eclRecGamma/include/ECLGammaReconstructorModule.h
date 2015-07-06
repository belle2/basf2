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
      bool goodGamma(double ftheta, double energy, double nhit, double fe9oe25, double fwidth);

      /** read Extrapolate CellID */
      void readExtrapolate();

    private:
      /** CPU time     */
      double m_timeCPU;
      /** Run number   */
      int m_nRun;
      /** Event number */
      int m_nEvent;
      /** extrapolated cell */
      bool m_TrackCellId[8736];
      /** good Gammacut m_ecut=0.02 */
      double m_ecut;
      /** good Gammacut m_e925cut=0.75 */
      double m_e925cut;
      /** good Gammacut m_widcut=6.0 */
      double m_widcut;
      /** good Gammacut m_nhcut=0 */
      double m_nhcut;
      /** good Gammacut m_thetaMin=17 */
      double m_thetaMin;
      /** good Gammacut m_thetaMax=150 */
      double m_thetaMax;
    };
  }//ECL
}//belle2

#endif
