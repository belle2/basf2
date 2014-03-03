/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vishal     (ECL Software Group)                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCLUSTER_H
#define ECLCLUSTER_H

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

  /*! Class to store ECLCluster reconstructed from ECLDigit as Belle method
   * relation to ECLShower
   * filled in ecl/modules/eclRecShower/src/ECLReconstructorModule.cc
   */

  class ECLCluster : public RelationsObject {
  public:
    /**
     *default constructor : all values are set to 0
     */
    ECLCluster() {

      m_Energy = 0;   /**< Corrected energy (GeV) */
      m_EnedepSum = 0; /**< Uncorrected Energy Deposited in a shower (GeV) */
      m_Px = 0;       /**< Px Momentum of Gamma (GeV/c) */
      m_Py = 0;       /**< Py Momentum of Gamma (GeV/c) */
      m_Pz = 0;       /**< Pz Momentum of Gamma (GeV/c) */

      m_E9oE25 = 0;   /**< E9/E25: photon-like & isolation cut */


      // TODO
      m_Timing = 0;   /**< Timing information (at present study going on)*/
      m_ErrorTiming = 0;  /**< Timing Error (at present study going on)*/
      m_HighestE = 0; /**< Highest Energy stored in a Crystal in a shower (GeV) */
      m_LAT = 0; /**< Lateral distribution parameter (Not available now) */

      m_NofCrystals = 0; /**< Number of Crystals in a shower (Not available now) */
      m_Pi0Likelihood = 0; /**< Pi0 Likelihood (greater than 200 MeV) (Not available now)*/
      m_EtaLikelihood = 0; /**< Eta Likelihood (greater than 200 MeV) (Not available now) */
      m_NofTracks = 0; /**< Number of Tracks related to this shower (being studied)*/
      m_deltaL = 0; /**< arXiv:0711.1593 (Not available now)*/
      m_beta = 0; /**< arXiv:0711.1593 (Not available now)*/
      m_CrystHealth = 0; /**< Crystal Health (Not available now)
          -100 : healthy
          10 : Bad
          20 : Broken
          30 : Dead  */
      m_HimomentumPi0 = 0; /**< Likelihood of being High momentum Pi0 (Not available now)
            0 to 1 : Can be high momentum pi0
         */


    }



    // Set the variables down here

    /*! Set Corrected Energy (GeV)
     */
    void setEnergy(float Energy) {m_Energy = Energy;}

    /*! Set X component of Momentum (GeV/c)
     */
    void setPx(float Px) { m_Px = Px;}

    /*! Set Y component of Momentum (GeV/c)
     */
    void setPy(float Py) { m_Py = Py;}

    /*! Set Z component of Momentum (GeV/c)
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

    /*! Set  Error Matrix (7x7)  for momentum TMatrixFSym class
     */
    void setErrorMatrix(TMatrixFSym& errorMatrix) {
      m_errorMatrix.ResizeTo(7, 7);
      for (int i = 0; i < 7; i++) {
        for (int j = 0; j <= i ; j++) {
          m_errorMatrix[i][j] = errorMatrix[i][j];
        }
      }
    }




    /*! Set Highest Energy stored in a Crystal in a shower (GeV)
     */
    void setHighestE(float HighestE) { m_HighestE = HighestE; }


    /*! Set Lateral distribution parameter
     */
    void setLAT(float LAT) {m_LAT = LAT;}

    /*! Set Number of Crystals in a shower
     */
    void setNofCrystals(int NofCrystals) {m_NofCrystals = NofCrystals;}

    /*! Set Pi0 Likelihood
     */
    void setPi0Likelihood(float Pi0Likelihood) {
      m_Pi0Likelihood = Pi0Likelihood;
    }

    /*! Set Eta Likelihood
     */
    void setEtaLikelihood(float EtaLikelihood) {
      m_EtaLikelihood = EtaLikelihood;
    }

    /*! Set Number of charged tracks identified with corresponsing shower
     */
    void setNofTracks(int NofTracks) {
      m_NofTracks = NofTracks;
    }

    /*! Set Delta L for charged track
     */
    void setdeltaL(float deltaL) {
      m_deltaL = deltaL;
    }

    /*! Set beta  for charged tracks
     */
    void setbeta(float beta) {
      m_beta = beta;
    }

    /*! Set Crystal health (Not now)
     */
    void setCrystHealth(int CrystHealth) {
      m_CrystHealth = CrystHealth;
    }

    /*! Set high momentum pi0 likelihood
     */
    void setHimomentumPi0(float HimomentumPi0) {
      m_HimomentumPi0 = HimomentumPi0;
    }





    // Get the value


    /*! Return Correct Energy (GeV)
     */
    float getEnergy() const {return m_Energy;}

    /*! Return X component of Momentum (GeV/c)
     */
    float getPx() const {return m_Px;}

    /*! Return Y component of Momentum (GeV/c)
     */
    float getPy() const { return m_Py;}

    /*! Return Z component of Momentum (GeV/c)
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

    /*! Return HighestEnergy in a crystal in a shower
     */
    float getHighestE() const {return m_HighestE; }


    /*! Return LAT (shower variable)
     */
    float getLAT() const {return m_LAT;}

    /*! Return Number of a Crystals in a shower
     */
    int getNofCrystals() const {return m_NofCrystals;}

    /*! Return Pi0 Likelihood  of a shower
     */
    float getPi0Likelihood() const { return m_Pi0Likelihood; }

    /*! Return Eta Likelihood  of a shower
     */
    float getEtaLikelihood() const { return m_EtaLikelihood;}


    /*! Return Number of charged tracks identified with corresponsing shower
     */
    int getNofTracks() const { return m_NofTracks;}

    /*! Return Delta L for charged track
     */
    float getdeltaL() const { return m_deltaL;}

    /*! Set Return  for charged tracks
     */
    float getbeta() const { return m_beta;}

    /*! Return Crystal health (Not now)
     */
    int getCrystHealth() const { return m_CrystHealth;}


    /*! Return high momentum pi0 likelihood
     */
    float getHimomentumPi0() const { return m_HimomentumPi0;}

    // The method to get return TMatrixFSym  7 Momentum Error Matrix
    /*!   TMatrixFSym  7 Momentum Error Matrix
     * @return Error Matrix from E, theta, phi and their errors
     */
    TMatrixFSym getErrorMatrix() const { return m_errorMatrix;}








  private:

    TMatrixFSym m_errorMatrix;   /**< Error Matrix 7x7 */


    float  m_Energy;     /**< Corrected energy (GeV) */
    float  m_Px;         /**< Px Momentum of Gamma (GeV/c) */
    float  m_Py;         /**< Py Momentum of Gamma (GeV/c) */
    float  m_Pz;         /**< Pz Momentum of Gamma (GeV/c) */
    float  m_EnedepSum;  /**< Uncorrected  Energy Deposited (GeV) */
    float  m_Timing;     /**< Timing information (study going on) */
    float  m_ErrorTiming;    /**< Timing Error (NA) */
    float  m_E9oE25;     /**< E9/E25: photon-like & isolation cut */
    float  m_HighestE;  /**< Highest energy stored in a crystal in Shower */
    //TODO
    float m_LAT;  /**< Lateral distribution parameter (Not available now) */
    int   m_NofCrystals; /**< Number of Crystals in a shower (Not available now) */
    float m_Pi0Likelihood; /**< Pi0 Likelihood (greater than 200 MeV) (Not available now)*/
    float m_EtaLikelihood; /**< Eta Likelihood (greater than 200 MeV) (Not available now) */
    int   m_NofTracks; /**< Number of Tracks related to this shower (being studied)*/
    float m_deltaL; /**< DeltaL charged track arXiv:0711.1593 (Not available now)*/
    float m_beta; /**< Beta charged track arXiv:0711.1593 (Not available now)*/
    int   m_CrystHealth;/**< Crystal Health (Not available now)
          -100 : healthy
          10 : Bad
          20 : Broken
          30 : Dead  */
    float m_HimomentumPi0; /**< Likelihood of being High momentum Pi0 (Not available now) */



    /**< Class definition */
    ClassDef(ECLCluster, 1); /**< Needed to make objects storable */


  };

}// end namespace Belle2

#endif


