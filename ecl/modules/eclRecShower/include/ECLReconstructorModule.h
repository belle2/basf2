/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLRECONSTRUCTORMODULE_H_
#define ECLRECONSTRUCTORMODULE_H_

#include <framework/core/Module.h>
#include <vector>
#include <TRandom3.h>

namespace Belle2 {
  namespace ECL {
    class ECLReconstructorModule : public Module {

    public:

      /** Constructor.
       */
      ECLReconstructorModule();

      /** Destructor.
       */
      ~ECLReconstructorModule();


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
      /** members of ECLReconstructor Module
       */
      /** CPU time     */
      double m_timeCPU;
      /** Run number   */
      int    m_nRun;
      /** Event number */
      int    m_nEvent;
      /** ECLReconstructor index */
      int    m_hitNum;
      /**  ECLHitAssignment. index */
      int    m_HANum;
      /** function of sruare */
      double squ(double x) { return x * x; }

      /**calculate error of Energy with Energy  */
      float errorE(double E);

      /**calculate error of Theta with Energy and Theta  */
      float errorTheta(double Energy, double Theta);

      /**calculate error of Phi with Energy and Theta  */
      float errorPhi(double Energy, double Theta);

      /**calculate correction factor of energy depending on Energy and Theta  */
      double correctionFactor(double Energy, double Theta);




    };
  }//ECL
}//Belle2

#endif
