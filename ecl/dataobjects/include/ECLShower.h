/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWER_H
#define ECLSHOWER_H

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <math.h>

namespace Belle2 {

  /*! Class to store ECL Showers which are reconstructed from ECLDigit as Belle method
   * relation to ECLDigit
   * filled in ecl/modules/eclRecShower/src/ECLReconstructorModule.cc
   */

  class ECLShower : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLShower() {
      m_ShowerId = 0;      /**< Shower ID */
      m_Energy = 0;      /**< Energy (GeV) */
      m_Theta = 0;       /**< Theta (rad) */
      m_Phi = 0;         /**< Phi (rad) */
      m_R = 0;           /**< R inherit from Belle */
      m_Error[0] = 0;    /**< Error of Energy, Theta and Phi */
      m_Error[1] = 0;    /**< Error of Energy, Theta and Phi */
      m_Error[2] = 0;    /**< Error of Energy, Theta and Phi */
      m_Mass = 0;        /**< Mass, inherit from Belle */
      m_Width = 0;       /**< Width, inherit from Belle */
      m_E9oE25 = 0;      /**< E9oE25, inherit from Belle */
      m_E9oE25unf = 0;   /**< E9oE25unf, inherit from Belle */
      m_NHits = 0;       /**< NHits, inherit from Belle */
      m_Status = 0;        /**< Status, inherit from Belle */
      m_Grade = 0;         /**< Grade, inherit from Belle */
      m_UncEnergy = 0;   /**< UncEnergy, inherit from Belle */
      m_Time = 0;        /**< Time, new parameter for Belle2, wait for more stude */
    }


    /*! Set Shower ID
     */
    void setShowerId(int ShowerId) { m_ShowerId = ShowerId; }
    /*! Set Energy
     */
    void setEnergy(float Energy) { m_Energy = Energy; }
    /*! Set Theta (rad)
     */
    void setTheta(float Theta) { m_Theta = Theta; }
    /*! Set Phi (rad)
     */
    void setPhi(float Phi) { m_Phi = Phi; }
    /*! Set R
     */
    void setR(float R) { m_R = R; }
    /*! Set  Error of E, theta, phi
     */
    void setError(float  ErrorArray[3]) { for (int i = 0; i < 3; i++) { m_Error[i] = ErrorArray[i];} }
    /*! Set Mass
     */
    void setMass(float Mass) { m_Mass = Mass; }
    /*! Set Width
     */
    void setWidth(float Width) { m_Width = Width; }
    /*! Set E9oE25
     */
    void setE9oE25(float E9oE25) { m_E9oE25 = E9oE25; }
    /*! Set E9oE25unf
     */
    void setE9oE25unf(float E9oE25unf) { m_E9oE25unf = E9oE25unf; }
    /*! Set NHits
     */
    void setNHits(float NHits) { m_NHits = NHits; }
    /*! Set Status
     */
    void setStatus(int Status) { m_Status = Status; }
    /*! Set Grade
     */
    void setGrade(int Grade) { m_Grade = Grade ; }
    /*! Set UncEnergy
     */
    void setUncEnergy(float UncEnergy) { m_UncEnergy = UncEnergy; }
    /*! Set Time
     */
    void setTime(float Time) { m_Time = Time; }

    /*! Get Shower Id
     * @return Shower Id
     */
    int GetShowerId() const { return m_ShowerId ; }
    /*! Get Energy
     * @return Energy
     */
    float GetEnergy() const { return m_Energy ; }
    /*! Get Theta
     * @return Theta
     */
    float GetTheta() const { return m_Theta ; }
    /*! Get Phi
     * @return Phi
     */
    float GetPhi() const { return m_Phi ; }
    /*! Get R
     * @return R
     */
    float GetR() const { return m_R ; }
    /*! Get Error of E, theta, phi
     * @return Error E, theta, phi
     */
    void GetError(float  ErrorArray[3]) { for (int i = 0; i < 3; i++) { ErrorArray[i] = m_Error[i] ;} }
    /*! Get Error of Energy
     * @return Error of Energy
     */
    float GetEnergyError() { return m_Error[0] ;}
    /*! Get Error of theta
     * @return Error of theta
     */
    float GetThetaError() { return  m_Error[1] ;}
    /*! Get Error of phi
     * @return Error of phi
     */
    float GetPhiError() {return m_Error[2] ;}
    /*! Get Mass
     * @return Mass
     */
    float GetMass() const { return m_Mass ; }
    /*! Get Width
     * @return Width
     */
    float GetWidth() const { return m_Width ; }
    /*! Get E9oE25
     * @return E9oE25
     */
    float GetE9oE25() const { return m_E9oE25 ; }
    /*! Get E9oE25unf
     * @return E9oE25unf
     */
    float GetE9oE25unf() const { return m_E9oE25unf ; }
    /*! Get NHits
     * @return NHits
     */
    float GetNHits() const { return m_NHits ; }
    /*! Get Status
     * @return Status
     */
    int GetStatus() const { return m_Status ; }
    /*! Get Grade
     * @return Grade
     */
    int GetGrade() const { return m_Grade  ; }
    /*! Get UncEnergy
     * @return UncEnergy
     */
    float GetUncEnergy() const { return m_UncEnergy ; }
    /*! Get Time
     * @return Time
     */
    float GetTime() const { return m_Time ; }


    /*! Get Shower Id
     * @return Shower Id
     */
    int getShowerId() const { return m_ShowerId ; }
    /*! Get Energy
     * @return Energy
     */
    float getEnergy() const { return m_Energy ; }
    /*! Get Theta
     * @return Theta
     */
    float getTheta() const { return m_Theta ; }
    /*! Get Phi
     * @return Phi
     */
    float getPhi() const { return m_Phi ; }
    /*! Get R
     * @return R
     */
    float getR() const { return m_R ; }
    /*! Get Error of E, theta, phi
     * @return Error E, theta, phi
     */
    void getError(float  ErrorArray[3]) { for (int i = 0; i < 3; i++) { ErrorArray[i] = m_Error[i] ;} }
    /*! Get Error of Energy
     * @return Error of Energy
     */
    float getEnergyError() { return m_Error[0] ;}
    /*! Get Error of theta
     * @return Error of theta
     */
    float getThetaError() { return  m_Error[1] ;}
    /*! Get Error of phi
     * @return Error of phi
     */
    float getPhiError() {return m_Error[2] ;}
    /*! Get Mass
     * @return Mass
     */
    float getMass() const { return m_Mass ; }
    /*! Get Width
     * @return Width
     */
    float getWidth() const { return m_Width ; }
    /*! Get E9oE25
     * @return E9oE25
     */
    float getE9oE25() const { return m_E9oE25 ; }
    /*! Get E9oE25unf
     * @return E9oE25unf
     */
    float getE9oE25unf() const { return m_E9oE25unf ; }
    /*! Get NHits
     * @return NHits
     */
    float getNHits() const { return m_NHits ; }
    /*! Get Status
     * @return Status
     */
    int getStatus() const { return m_Status ; }
    /*! Get Grade
     * @return Grade
     */
    int getGrade() const { return m_Grade  ; }
    /*! Get UncEnergy
     * @return UncEnergy
     */
    float getUncEnergy() const { return m_UncEnergy ; }
    /*! Get Time
     * @return Time
     */
    float getTime() const { return m_Time ; }




    //! The method to get return  TVector3 Momentum
    TVector3 getMomentum() const {
      TVector3 momentum(0., 0., 0.);
      double m_px = m_Energy * sin(m_Theta) * cos(m_Phi);
      double m_py = m_Energy * sin(m_Theta) * sin(m_Phi);
      double m_pz = m_Energy * cos(m_Theta);

      momentum.SetX(m_px);
      momentum.SetY(m_py);
      momentum.SetZ(m_pz);
      return momentum;
    }


  private:
    int m_ShowerId;        /**< Shower ID */
    float m_Energy;        /**< Energy (GeV) */
    float m_Theta;         /**< Theta (rad) */
    float m_Phi;           /**< Phi (rad) */
    float m_R;             /**< R inherit from Belle */
    float m_Error[3];      /**< Error of Energy, Theta and Phi */
    float m_Mass;          /**< Mass, inherit from Belle */
    float m_Width;         /**< Width, inherit from Belle */
    float m_E9oE25;        /**< E9oE25, inherit from Belle */
    float m_E9oE25unf;     /**< E9oE25unf, inherit from Belle */
    float m_NHits;         /**< NHits, inherit from Belle */
    int m_Status;          /**< Status, inherit from Belle */
    int m_Grade;           /**< Grade, inherit from Belle */
    float m_UncEnergy;     /**< UncEnergy, inherit from Belle */
    float m_Time;          /**< Time, new parameter for Belle2, wait for more stude */

    ClassDef(ECLShower, 1);/**< ClassDef */

  };
} // end namespace Belle2

#endif
