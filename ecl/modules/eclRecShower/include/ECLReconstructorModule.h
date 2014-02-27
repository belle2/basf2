/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen,Vishal                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLRECONSTRUCTORMODULE_H_
#define ECLRECONSTRUCTORMODULE_H_

#include <framework/core/Module.h>
#include <vector>
#include <TRandom3.h>
#include <TMatrixFSym.h>

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

      /** Fill error matrix for Px, Py Pz and E */
      void readErrorMatrix(double Energy, double Theta,
                           double Phi, TMatrixFSym& ErrorMatrix);

      /** Fill error matrix for Px, Py Pz and E */
      void readErrorMatrix7x7(double Energy, double Theta,
                              double Phi, TMatrixFSym& ErrorMatrix);

      /** read Extrapolate CellID */
      void readExtrapolate();

      /**calculate Px using Energy, Theta and Phi */
      float Px(double Energy, double Theta, double Phi);

      /** Calculate Py using Energy, Theta and Phi */
      float Py(double Energy, double Theta, double Phi);

      /** Calculate Pz using Energy, Theta  */
      float Pz(double Energy, double Theta);

      /** extrapolated cell */
      int   m_TrackCellId[8736];


    };
  }//ECL
}//Belle2

#endif
