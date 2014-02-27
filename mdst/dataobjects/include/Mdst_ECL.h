/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vishal     (ECL Software Group)                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MDST_ECL_H
#define MDST_ECL_H

#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <cmath>
#include <TLorentzVector.h>
#include <TMatrixFSym.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /*! Class to store Mdst_ECL reconstructed from ECLDigit as Belle method
   * relation to ECLShower
   * filled in ecl/modules/eclRecShower/src/ECLReconstructorModule.cc
   */

  class Mdst_ECL : public RelationsObject {
  public:
    /**
     *default constructor : all values are set to 0
     */
    Mdst_ECL() {

      m_Energy = 0;   /**< Corrected energy (GeV) */
      m_EnedepSum = 0; /**< Uncorrected Energy Deposited in a shower (GeV) */
      m_Px = 0;       /**< Px Momentum of Gamma (GeV) */
      m_Py = 0;       /**< Py Momentum of Gamma (GeV) */
      m_Pz = 0;       /**< Pz Momentum of Gamma (GeV) */

      m_E9oE25 = 0;   /**< E9/E25: photon-like & isolation cut */


      // TODO
      m_Timing = 0;   /**< Timing information */
      m_ErrorTiming = 0;  /**< Timing Error */
      m_HighestE = 0; /**< Highest Energy stored in a Crystal in a shower (GeV) */
      m_LAT = 0; /**< Lateral distribution parameter */

      m_NofCrystals = 0; /**< Number of Crystals in a shower */
      m_Pi0Likelihood = 0; /**< Pi0 Likelihood (greater than 200 MeV) */
      m_EtaLikelihood = 0; /**< Eta Likelihood (greater than 200 MeV) */
      m_NofTracks = 0; /**< Number of Tracks related to this shower */
      m_deltaL = 0; /**< arXiv:0711.1593 */
      m_beta = 0; /**< arXiv:0711.1593 */
      m_CrystHealth = 0; /**< Crystal Health
        -100 : healthy
        10 : Bad
        20 : Broken
        30 : Dead  */
      m_HimomentumPi0 = 0; /**< Likelihood of being High momentum Pi0
          0 to 1 : Can be high momentum pi0
       */


    }

    // Function to return Good Gamma
    /*! Return  */


    // Set the variables down here

    /*! Set Correct Energy (GeV)
     */
    void setEnergy(float Energy) {m_Energy = Energy;}

    /*! Set X component of Momentum (GeV)
     */
    void setPx(float Px) { m_Px = Px;}

    /*! Set Y component of Momentum (GeV)
     */
    void setPy(float Py) { m_Py = Py;}

    /*! Set Z component of Momentum (GeV)
     */
    void setPz(float Pz) { m_Pz = Pz;}

    /*! Set Uncorrect Energy deposited (GeV)
     */
    void setEnedepSum(float EnedepSum) {m_EnedepSum = EnedepSum;}

    /*! Set Timing information
     */
    void setTiming(float Timing) {m_Timing = Timing;}

    /*! Set Error on Timing information
     */
    void setErrorTiming(float ErrorTiming) {m_ErrorTiming = ErrorTiming;}

    /*! Set E9/E25
     */
    void setE9oE25(float E9oE25) { m_E9oE25 = E9oE25; }

    /*! Set  Error Matrix  for momentum TMatrixFSym class
     */
    void setErrorMatrix(TMatrixFSym& errorMatrix) {
      m_errorMatrix.ResizeTo(7, 7);
      for (int i = 0; i < 7; i++) {
        for (int j = 0; j <= i ; j++) {
          m_errorMatrix[i][j] = errorMatrix[i][j];
        }
      }
    }




    /*! Set HighestE
     */
    void setHighestE(float HighestE) { m_HighestE = HighestE; }



    void setLAT(float LAT) {m_LAT = LAT;}
    void setNofCrystals(int NofCrystals) {m_NofCrystals = NofCrystals;}
    void setPi0Likelihood(float Pi0Likelihood) {
      m_Pi0Likelihood = Pi0Likelihood;
    }
    void setEtaLikelihood(float EtaLikelihood) {
      m_EtaLikelihood = EtaLikelihood;
    }

    void setNofTracks(int NofTracks) {
      m_NofTracks = NofTracks;
    }

    void setdeltaL(float deltaL) {
      m_deltaL = deltaL;
    }

    void setbeta(float beta) {
      m_beta = beta;
    }

    void setCrystHealth(int CrystHealth) {
      m_CrystHealth = CrystHealth;
    }

    void setHimomentumPi0(float HimomentumPi0) {
      m_HimomentumPi0 = HimomentumPi0;
    }





    // Get the value


    /*! Return Correct Energy (GeV)
     */
    float getEnergy() const {return m_Energy;}

    /*! Return X component of Momentum (GeV)
     */
    float getPx() const {return m_Px;}

    /*! Return Y component of Momentum (GeV)
     */
    float getPy() const { return m_Py;}

    /*! Return Z component of Momentum (GeV)
     */
    float getPz() const { return m_Pz;}

    /*! Return Uncorrect Energy deposited (GeV)
     */
    float getEnedepSum() const {return m_EnedepSum;}

    /*! Return Timing information
     */
    float getTiming() const {return m_Timing;}

    /*! Return Error on Timing information
     */
    float getErrorTiming() const {return m_ErrorTiming;}

    /*! Return E9/E25
     */
    float getE9oE25() const { return m_E9oE25; }

    /*! Return HighestE
     */
    float getHighestE() const {return m_HighestE; }


    float getLAT() const {return m_LAT;}
    int getNofCrystals() const {return m_NofCrystals;}
    float getPi0Likelihood() const { return m_Pi0Likelihood; }
    float getEtaLikelihood() const { return m_EtaLikelihood;}

    int getNofTracks() const { return m_NofTracks;}

    float getdeltaL() const { return m_deltaL;}

    float getbeta() const { return m_beta;}

    int getCrystHealth() const { return m_CrystHealth;}

    float getHimomentumPi0() const { return m_HimomentumPi0;}

    //! The method to get return TMatrixFSym  7 Momentum Error Matrix
    /*! Get  TMatrixFSym  7 Momentum Error Matrix
     * @return Error Matrix from E, theta, phi and their errors
     */
    TMatrixFSym getErrorMatrix() const { return m_errorMatrix;}








  private:

    TMatrixFSym m_errorMatrix;   /**< Error Matrix 7x7 */


    float  m_Energy;     /**< Corrected energy (GeV) */
    float  m_Px;         /**< Px Momentum of Gamma (GeV) */
    float  m_Py;         /**< Py Momentum of Gamma (GeV) */
    float  m_Pz;         /**< Pz Momentum of Gamma (GeV) */
    float  m_EnedepSum;  /**< Uncorrected  Energy Deposited (GeV) */
    float  m_Timing;     /**< Timing information */
    float  m_ErrorTiming;    /**< Timing Error */
    float  m_E9oE25;     /**< E9/E25: photon-like & isolation cut */
    float  m_HighestE;  /**< Highest energy stored in a crystal in Shower */

    float m_LAT;
    int   m_NofCrystals;
    float m_Pi0Likelihood;
    float m_EtaLikelihood;
    int   m_NofTracks;
    float m_deltaL;
    float m_beta;
    int   m_CrystHealth;
    float m_HimomentumPi0;



    /**< Class definition */
    ClassDef(Mdst_ECL, 1);


  };

}// end namespace Belle2

#endif


