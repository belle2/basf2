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
  class ECLCluster;
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

      /** function of sruare */
      double squ(double x) { return x * x; }

      /**calculate error of Energy with Energy  */
      double errorEnergy(double energy);

      /**calculate error of Theta with Energy and Theta  */
      double errorTheta(double energy, double theta);

      /**calculate error of Phi with Energy and Theta  */
      double errorPhi(double energy, double theta);

      /**calculate correction factor of energy depending on Energy and Theta  */
      double correctionFactor(double energy, double theta);

      /** Fill error matrix for Px, Py Pz and E */
      void readErrorMatrix(double energy, double theta,
                           double phi, TMatrixFSym& errorMatrix);

      /** Fill error matrix for Px, Py Pz and E */
      void readErrorMatrix7x7(double energy, double theta,
                              double phi, TMatrixFSym& errorMatrix);

      /**calculate Px using Energy, Theta and Phi */
      double Px(double energy, double theta, double phi);

      /** Calculate Py using Energy, Theta and Phi */
      double Py(double energy, double theta, double phi);

      /** Calculate Pz using Energy, Theta  */
      double Pz(double energy, double theta);

      // read correction accounting shower leakage to get unbiased photon energy
      void ReadCorrection();

      // m_ranges -- theta ranges for the correction
      // m_ecorr  -- correction polynomial coefficients storage
      std::vector<double> m_ranges, m_ecorr;

      //Temp fix to correct energy bias
      //to be removed ASAP with a proper calibration

      class TmpClusterCorrection {
      public:
        void init(const std::string& filename);
        void scale(Belle2::ECLCluster& c) const;
      private:
        double m_deltaE;
        std::size_t m_npointsE;
        std::vector<double> m_maxTheta;
        std::vector<double> m_tmpCorrection;
      };

      TmpClusterCorrection m_tmpClusterCorrection;

    };
  }//ECL
}//Belle2

#endif
