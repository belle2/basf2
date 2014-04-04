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
      m_isTrack = false; /**< To store relation to tracks [true] or not [false] (for timebeing, Pleaseuse function  isNeutral() [as it is for future] */
      m_Energy = 0;   /**< Corrected energy (GeV) */
      m_EnedepSum = 0; /**< Uncorrected Energy Deposited in a shower (GeV) */
      m_Theta = 0;     /**< Theta (in radian) */
      m_Phi = 0;       /**< Phi (in radian) */
      m_R = 0;        /**<  R (in cm) */
      m_E9oE25 = 0;   /**< E9/E25: photon-like & isolation cut */

      // TODO
      m_Timing = 0;   /**< Timing information (at present study going on)*/
      m_ErrorTiming = 0;  /**< Timing Error (at present study going on)*/
      m_HighestE = 0; /**< Highest Energy stored in a Crystal in a shower (GeV) */
      m_LAT = 0; /**< Lateral distribution parameter (Not available now) */
      m_NofCrystals = 0; /**< Number of Crystals in a shower (Not available now) */
      m_CrystHealth = 0; /**< Crystal Health (Not available now)
          -100 : healthy
          10 : Bad
          20 : Broken
          30 : Dead  */
      m_MergedPi0 = 0; /**< Likelihood of being Merged Pi0
            (Not available now)
            0 to 1 : Can be a Merged Pi0
         */

      m_Error[0] = 0;
      m_Error[1] = 0;
      m_Error[2] = 0;
      m_Error[3] = 0;
      m_Error[4] = 0;
      m_Error[5] = 0;

    }



    // Set the variables down here

    /*! Set Corrected Energy (GeV)
     */
    void setEnergy(float Energy) {m_Energy = Energy;}

    /*! Set Theta of Shower (radian)
     */
    void setTheta(float Theta) { m_Theta = Theta;}

    /*! Set Phi of Shower (radian)
     */
    void setPhi(float Phi) { m_Phi = Phi;}

    /*! Set R (in cm)
     */
    void setR(float R) { m_R = R; }

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



    /*! Set Highest Energy stored in a Crystal in a shower (GeV)
     */
    void setHighestE(float HighestE) { m_HighestE = HighestE; }


    /*! Set Lateral distribution parameter
     */
    void setLAT(float LAT) {m_LAT = LAT;}

    /*! Set Number of Crystals in a shower
     */
    void setNofCrystals(int NofCrystals) {m_NofCrystals = NofCrystals;}


    /*! Set Crystal health (Not now)
     */
    void setCrystHealth(int CrystHealth) {
      m_CrystHealth = CrystHealth;
    }

    /*! Set high momentum pi0 likelihood
     */
    void setMergedPi0(float MergedPi0) {
      m_MergedPi0 = MergedPi0;
    }


    /*! Set  Error Array(3x3)  for
      [0]->Error on Energy
      [2]->Error on Phi
      [5]->Error on Theta
    */
    void setError(float ErrorArray[6]) {
      for (int i = 0; i < 6; ++i) {
        m_Error[i] = ErrorArray[i];
      }
    }


    /*! Set m_isTrack true if the cluster matches with cluster
     */
    void setisTrack(bool istrack) {
      m_isTrack = istrack;
    }


    // Get the value


    /*! Return Correct Energy (GeV)
     */
    float getEnergy() const {return m_Energy;}

    /*! Return Theta of Shower (radian)
     */
    float getTheta() const {return m_Theta;}

    /*! Return Phi of Shower (radian)
     */
    float getPhi() const { return m_Phi;}
    /*! Return R
     */
    float getR() const { return m_R ; }

    /*! Return Error on Energy
     */
    float getErrorEnergy() const {return m_Error[0];}

    /*! Return Error on Theta of Shower
     */
    float getErrorTheta() const {return m_Error[5];}

    /*! Return Error on Phi of Shower
     */
    float getErrorPhi() const { return m_Error[2];}


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


    /*! Return Crystal health (Not now)
     */
    int getCrystHealth() const { return m_CrystHealth;}


    /*! Return high momentum pi0 likelihood
     */
    float getMergedPi0() const { return m_MergedPi0;}

    // The method to get return TMatrixFSym  7 Momentum Error Matrix



    //................
    // Return here Px, Py and Pz

    /*! Return Px (GeV/c)
     */
    float getPx() {
      return float(m_Energy * sin(m_Theta) * cos(m_Phi));
    }

    /*! Return Py (GeV/c)
     */
    float getPy() {
      return float(m_Energy * sin(m_Theta) * sin(m_Phi));
    }

    /*! Return Pz (GeV/c)
     */
    float getPz() {
      return float(m_Energy * cos(m_Theta));
    }

    ///.......


    /*! Return TVector3 momentum (Px,Py,Pz)
     */
    TVector3 getMomentum() const {

      return TVector3(float(m_Energy * sin(m_Theta) * cos(m_Phi)), float(m_Energy * sin(m_Theta) * sin(m_Phi)), float(m_Energy * cos(m_Theta)));
    }



    /*! Return 4Vector  (Px,Py,Pz,E)
     */
    TLorentzVector get4Vector() const {
      return TLorentzVector(float(m_Energy * sin(m_Theta) * cos(m_Phi)), float(m_Energy * sin(m_Theta) * sin(m_Phi)), float(m_Energy * cos(m_Theta)), m_Energy);
    }



    /*! Return TVector3 on cluster position /Shower center (x,y,z)
     */
    TVector3 getclusterPosition() const {
      float cluster_x =  m_R * sin(m_Phi) * cos(m_Theta);
      float cluster_y =  m_R * sin(m_Phi) * sin(m_Theta);
      float cluster_z =  m_R * cos(m_Phi);
      return TVector3(cluster_x, cluster_y, cluster_z);
    }

    /*! Return TVector3 on  position on gamma's production
      By default the position of gamma's production is (0,0,0)
    */
    TVector3 getPosition() const {
      float x, y, z;
      x = y = z = 0.0;
      return TVector3(x, y, z);
    }


    //.................
    // For filling error matrix on Px,Py and Pz

    /*! Return TMatrixFsym 4x4  error matrix
    */

    TMatrixFSym getError4x4() const {
      TMatrixFSym errorecl(3);
      errorecl[0][0] = m_Error[0] * m_Error[0]; //Energy
      errorecl[1][0] = m_Error[1];
      errorecl[1][1] = m_Error[2] * m_Error[2]; // Phi
      errorecl[2][0] = m_Error[3];
      errorecl[2][1] = m_Error[4];
      errorecl[2][2] = m_Error[5] * m_Error[5]; // Theta

      TMatrixF  jacobian(4, 3);
      float  cosPhi = cos(m_Phi);
      float  sinPhi = sin(m_Phi);
      float  cosTheta = cos(m_Theta);
      float  sinTheta = sin(m_Theta);
      float   E = m_Energy;

      jacobian[ 0 ][ 0 ] =            cosPhi * sinTheta;
      jacobian[ 0 ][ 1 ] =  -1.0 * E * sinPhi * sinTheta;
      jacobian[ 0 ][ 2 ] =        E * cosPhi * cosTheta;
      jacobian[ 1 ][ 0 ] =            sinPhi * sinTheta;
      jacobian[ 1 ][ 1 ] =        E * cosPhi * sinTheta;
      jacobian[ 1 ][ 2 ] =        E * sinPhi * cosTheta;
      jacobian[ 2 ][ 0 ] =                     cosTheta;
      jacobian[ 2 ][ 1 ] =           0.0;
      jacobian[ 2 ][ 2 ] =  -1.0 * E          * sinTheta;
      jacobian[ 3 ][ 0 ] =           1.0;
      jacobian[ 3 ][ 1 ] =           0.0;
      jacobian[ 3 ][ 2 ] =           0.0;
      TMatrixFSym errCart(4);
      errCart = errorecl.Similarity(jacobian);
      return errCart;
    }
    //__________________________


    /*! Return TMatrixFsym 7x7  error matrix
     */
    TMatrixFSym getError7x7() const {
      TMatrixFSym errorecl(3);
      errorecl[0][0] = m_Error[0] * m_Error[0]; //Energy
      errorecl[1][0] = m_Error[1];
      errorecl[1][1] = m_Error[2] * m_Error[2]; // Phi
      errorecl[2][0] = m_Error[3];
      errorecl[2][1] = m_Error[4];
      errorecl[2][2] = m_Error[5] * m_Error[5]; // Theta

      TMatrixFSym ErrorMatrix(7);
      TMatrixF  jacobian(4, 3);
      float  cosPhi = cos(m_Phi);
      float  sinPhi = sin(m_Phi);
      float  cosTheta = cos(m_Theta);
      float  sinTheta = sin(m_Theta);
      float   E = m_Energy;

      jacobian[ 0 ][ 0 ] =            cosPhi * sinTheta;
      jacobian[ 0 ][ 1 ] =  -1.0 * E * sinPhi * sinTheta;
      jacobian[ 0 ][ 2 ] =        E * cosPhi * cosTheta;
      jacobian[ 1 ][ 0 ] =            sinPhi * sinTheta;
      jacobian[ 1 ][ 1 ] =        E * cosPhi * sinTheta;
      jacobian[ 1 ][ 2 ] =        E * sinPhi * cosTheta;
      jacobian[ 2 ][ 0 ] =                     cosTheta;
      jacobian[ 2 ][ 1 ] =           0.0;
      jacobian[ 2 ][ 2 ] =  -1.0 * E          * sinTheta;
      jacobian[ 3 ][ 0 ] =           1.0;
      jacobian[ 3 ][ 1 ] =           0.0;
      jacobian[ 3 ][ 2 ] =           0.0;
      TMatrixFSym errCart(4);
      errCart = errorecl.Similarity(jacobian);
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j <= i ; j++) {
          ErrorMatrix[i][j] = errCart[i][j];
        }
      }
      for (int i = 4; i <= 6; ++i) {
        ErrorMatrix[i][i] = 1.0; // 1.0*1.0 cm^2 (default treatment as Belle ?)
      }
      return ErrorMatrix;
    }






    /*! Return TMatrixFsym 3x3 error matrix for E, Phi and Theta
     */

    TMatrixFSym getError3x3() const {
      TMatrixFSym errorecl(3);
      errorecl[0][0] = m_Error[0] * m_Error[0]; //Energy
      errorecl[1][0] = m_Error[1];
      errorecl[1][1] = m_Error[2] * m_Error[2]; // Phi
      errorecl[2][0] = m_Error[3];
      errorecl[2][1] = m_Error[4];
      errorecl[2][2] = m_Error[5] * m_Error[5]; // Theta
      return errorecl;
    }

    /*! Return m_isTrack true if the cluster matches with cluster, otherwise
      false (for time being). Please use isNeutral function (as it will be kept
      for future)
     */
    bool getisTrack() const {
      return m_isTrack;
    }

    /*! Return true if cluster has no match with cluster, otherwise
      return false if cluster has match with track.
     */
    bool isNeutral() const {
      if (m_isTrack) {
        return false;
      } else { return true; }
    }


    //..... For FUTURE (to DO)

    /*! Return pi0Likelihood for a shower (for future, Not available now)
     */
    float getpi0Likelihood() {
      return 0.5;
    }
    /*! Return etaLikelihood for a shower (for future, Not available now)
     */
    float getetaLikelihood() {
      return 0.5;
    }

    /*! Return deltaL for shower shape (for future, Not available now)
     */
    float getdeltaL() {
      return 0;
    }
    /*! Return beta for shower shape (for future, Not available now)
     */
    float getbeta() {
      return 0;
    }




  private:

    float m_Error[6]; /**< Error Array for Energy->[0], Phi->[2], Theta->[5] */
    bool m_isTrack;  /**< To store relation to tracks or not (for timebeing, Pleaseuse function  isNeutral() [as it is for future] */
    float  m_Energy;     /**< Corrected energy (GeV) */
    float  m_Theta;         /**< Theta of Shower (radian) */
    float  m_Phi;         /**< Phi of Shower (radian)  */


    float  m_R;         /**< R (cm) */
    float  m_EnedepSum;  /**< Uncorrected  Energy Deposited (GeV) */
    float  m_Timing;     /**< Timing information (study going on) */
    float  m_ErrorTiming;    /**< Timing Error (NA) */
    float  m_E9oE25;     /**< E9/E25: photon-like & isolation cut */
    float  m_HighestE;  /**< Highest energy stored in a crystal in Shower */
    //TODO
    float m_LAT;  /**< Lateral distribution parameter (Not available now) */
    int   m_NofCrystals; /**< Number of Crystals in a shower (Not available now) */
    int   m_CrystHealth;/**< Crystal Health (Not available now)
          -100 : healthy
          10 : Bad
          20 : Broken
          30 : Dead  */
    float m_MergedPi0; /**< Likelihood of being Merged Pi0 (Not available now) */



    /**< Class definition */
    ClassDef(ECLCluster, 1); /**< Needed to make objects storable */


  };

}// end namespace Belle2

#endif


