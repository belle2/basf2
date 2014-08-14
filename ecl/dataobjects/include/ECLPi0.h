/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLPI0_H
#define ECLPI0_H

#include <framework/datastore/DataStore.h>
#include "ecl/dataobjects/ECLShower.h"

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>
#include <TMatrixFSym.h>
#include <TLorentzVector.h>


namespace Belle2 {

  /*! Class to store ECL Showers which are reconstructed from ECLDigit as Belle method
   * relation to ECLGamma
   * filled in ecl/modules/eclRecShower/src/ECLReconstructorModule.cc
   * modify the name of function as convention,
   * still keep old function getmassfit not in convention to avoid compiling error 2012.11.22
   */

  class ECLPi0 : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLPi0() {
      m_ShowerId1 = 0;     /**< Shower Id of higher enegry Gamma */
      m_ShowerId2 = 0;     /**< Shower Id of lower enegry Gamma */
      m_Energy = 0;      /**< Energy (GeV) */
      m_Px = 0;          /**< Px (GeV) */
      m_Py = 0;          /**< Py (GeV) */
      m_Pz = 0;          /**< Pz (GeV) */
      m_Mass = 0;        /**< combined Mass  (GeV) */
      m_MassFit = 0;     /**< MassFit (GeV) */
      m_Chi2 = 0;        /**< Chi square of fit (GeV) */
    }


    /*! Set Shower ID of higher energy gamma
     */
    void setShowerId1(int ShowerId1) { m_ShowerId1 = ShowerId1; }

    /*! Set Shower ID of lower energy gamma
     */
    void setShowerId2(int ShowerId2) { m_ShowerId2 = ShowerId2; }
    /*! Set Energy
     */
    void setEnergy(float Energy) { m_Energy = Energy; }
    /*! Set Px
     */
    void setPx(float Px) { m_Px = Px; }
    /*! Set Py
     */
    void setPy(float Py) { m_Py = Py; }
    /*! Set Pz
     */
    void setPz(float Pz) { m_Pz = Pz; }
    /*! Set Mass
     */
    void setMass(float Mass) { m_Mass = Mass; }
    /*! Set Mass after mass constraint fit
     */
    void setMassFit(float MassFit) { m_MassFit = MassFit; }
    /*! Set Chi square after mass constraint fit
     */
    void setChi2(float Chi2) { m_Chi2 = Chi2; }

    /*! Set Pi0 Error Matrix  after mass constraint fit by TMatrixFSym class
     */
    void setErrorMatrix(TMatrixFSym& errorMatrix) {
      m_errorMatrix.ResizeTo(4, 4);
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j <= i ; j++) {
          m_errorMatrix[i][j] = errorMatrix[i][j];
        }
      }
    }
    /*! Set Pi0 Error Matrix  after mass constraint fit by HepSymMatrix class
     */



    /*! Get Momentum after mass constraint fit
     * @return HepLorentzVector Momentum  after mass constraint fit
     */
    TLorentzVector get4Momentum() const {
      TLorentzVector  momentum(m_Px, m_Py, m_Pz, m_Energy);
      return momentum;
    }


    /*! Get ShowerId1
     * @return ShowerId1
     */
    int getShowerId1() const { return m_ShowerId1 ; }
    /*! Get ShowerId2
     * @return ShowerId2
     */
    int getShowerId2() const { return m_ShowerId2 ; }
    /*! Get Energy
     * @return Energy;
     */
    float getEnergy() const { return m_Energy; }
    /*! Get Px
     * @return Px
     */
    float getPx() const { return m_Px; }
    /*! Get Py
     * @return Py
     */
    float getPy() const { return m_Py; }
    /*! Get Pz
     * @return Pz
     */
    float getPz() const { return m_Pz; }
    /*! Get Mass
     * @return Mass
     */
    float getMass() const { return m_Mass; }
    /*! Get  Mass after mass constraint fit
     * @return  Mass after mass constraint fit
     */
    float getMassFit() const { return m_MassFit; }
    /*! Get Chi square after mass constraint fit
     * @return Chi square after mass constraint fit
     */
    float getChi2() const { return m_Chi2; }

    /*! Get pValue
     * @return pValue
     */
    float getPValue() const {

      double par[] = {5.43555e-01, -1.62126e-01, -4.31663e-03, 4.13831e-03, 9.07563e-05, -5.37161e-05};
      double pValue =
        par[0] +  par[1] * log(m_Chi2) +  par[2] * log(m_Chi2) * log(m_Chi2) + par[3] * log(m_Chi2) * log(m_Chi2) * log(m_Chi2)
        + par[4] * log(m_Chi2) * log(m_Chi2) * log(m_Chi2) * log(m_Chi2)
        + par[5] * log(m_Chi2) * log(m_Chi2) * log(m_Chi2) * log(m_Chi2) * log(m_Chi2);
      if (pValue > 1)pValue = 1;
      if (pValue < 0)pValue = 0;

      return pValue;
    }


    //! The method to get return TMatrixFSym  4 Momentum Error Matrix
    /*! Get  TMatrixFSym  4 Momentum Error Matrix
     * @return Error Matrix from particle Error propagation of 2 gammas
     */
    TMatrixFSym getErrorMatrix() const { return m_errorMatrix;}

    //! The method to get return TMatrixT<float>   4 Momentum Error Matrix
    /*! Get  TMatrixT<float>   4 Momentum Error Matrix by reading ECLShower through ShowerId
     * @return Matrix which from convertion from Error E, Theta, Phi of ECLShower
    */
    void getErrorMatrix(TMatrixFSym& errorMatrix) const {

      for (int i = 0; i < 4; i++) {
        for (int j = 0; j <= i ; j++) {
          errorMatrix[i][j] = m_errorMatrix[i][j];
          //std::cout<<m_errorMatrix[i][j]<<" ";
        }
      }
    }

    //! The method to get return TMatrixT<float>   4 Momentum Error Matrix
    /*! Get  TMatrixT<float>   4 Momentum and Vertex Error Matrix by reading ECLShower through ShowerId
     * @return Matrix which from convertion from Error E, Theta, Phi of ECLShower
    */
    void getErrorMatrix7x7(TMatrixFSym& errorMatrix) const {

      for (int i = 0; i < 4; i++) {
        for (int j = 0; j <= i ; j++) {
          errorMatrix[i][j] = m_errorMatrix[i][j];
          //std::cout<<m_errorMatrix[i][j]<<" ";
        }
      }
      errorMatrix[4][4] = 1.;
      errorMatrix[5][5] = 1.;
      errorMatrix[6][6] = 1.;

      //std::cout<<std::endl;
    }







  private:
    int m_ShowerId1;       /**< Shower Id of higher enegry Gamma */
    int m_ShowerId2;       /**< Shower Id of lower enegry Gamma */
    float m_Energy;        /**< Energy (GeV) */
    float m_Px;            /**< Px (GeV) */
    float m_Py;            /**< Py (GeV) */
    float m_Pz;            /**< Pz (GeV) */
    float m_Mass;          /**< combined Mass  (GeV) */
    float m_MassFit;       /**< MassFit (GeV) */
    float m_Chi2;          /**< Chi square of fit (GeV) */
    TMatrixFSym m_errorMatrix;   /**< Error Matrix 4x4 */

    ClassDef(ECLPi0, 1);/**< ClassDef */

  };
} // end namespace Belle2


#endif
