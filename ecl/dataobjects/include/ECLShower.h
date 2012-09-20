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

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>
#include <math.h>
namespace Belle2 {

  //! Example Detector
  class ECLShower : public TObject {
  public:
    /**shower id of this shower. */
    int m_showerId;
    /**Energy of this shower. */
    float m_Energy;
    /**Theta of this shower. */
    float m_Theta;
    /**Phi of this shower. */
    float m_Phi;
    /**Distance of this shower. */
    float m_R;
    /**Error of this shower. */
    float m_Error[6];
    /**Mass of this shower. */
    float m_Mass;
    /**Width of this shower. */
    float m_Width;
    /**E9oE25 of this shower. */
    float m_E9oE25;
    /**E9oE25unf of this shower. */
    float m_E9oE25unf;
    /**NHits of this shower. */
    float m_NHits;
    /**Status of this shower. */
    int m_Status;
    /**Grade of this shower. */
    int m_Grade;
    /**EUncEnergy of this shower. */
    float m_UncEnergy;
    /**Time of this shower. */
    float m_Time;


    //! The method to set showerId
    void setShowerId(int showerId) { m_showerId = showerId; }
    //! The method to set Energy
    void setEnergy(float Energy) { m_Energy = Energy; }
    //! The method to set Theta
    void setTheta(float Theta) { m_Theta = Theta; }
    //! The method to set Phi
    void setPhi(float Phi) { m_Phi = Phi; }
    //! The method to set Distance
    void setR(float R) { m_R = R; }
    //! The method to set Error
    void setError(float  ErrorArray[3]) { for (int i = 0; i < 3; i++) { m_Error[i] = ErrorArray[i];} }
    //! The method to set Mass
    void setMass(float Mass) { m_Mass = Mass; }
    //! The method to set Width
    void setWidth(float Width) { m_Width = Width; }
    //! The method to set E9oE25
    void setE9oE25(float E9oE25) { m_E9oE25 = E9oE25; }
    //! The method to set E9oE25unf
    void setE9oE25unf(float E9oE25unf) { m_E9oE25unf = E9oE25unf; }
    //! The method to set NHits
    void setNHits(float NHits) { m_NHits = NHits; }
    //! The method to set Status
    void setStatus(int Status) { m_Status = Status; }
    //! The method to set Grade
    void setGrade(int Grade) { m_Grade = Grade ; }
    //! The method to set m_UncEnergy
    void setUncEnergy(float UncEnergy) { m_UncEnergy = UncEnergy; }
    //! The method to set m_Time
    void setTime(float Time) { m_Time = Time; }

    //! The method to get showerId
    int GetShowerId() const { return m_showerId ; }
    //! The method to get Energy
    float GetEnergy() const { return m_Energy ; }
    //! The method to get Theta
    float GetTheta() const { return m_Theta ; }
    //! The method to get Phi
    float GetPhi() const { return m_Phi ; }
    //! The method to get Distance
    float GetR() const { return m_R ; }
    //! The method to get Error
    void GetError(float  ErrorArray[6]) { for (int i = 0; i < 6; i++) { ErrorArray[i] = m_Error[i] ;} }
    //! The method to get Mass
    float GetMass() const { return m_Mass ; }
    //! The method to get Width
    float GetWidth() const { return m_Width ; }
    //! The method to get E9oE25
    float GetE9oE25() const { return m_E9oE25 ; }
    //! The method to get E9oE25unf
    float GetE9oE25unf() const { return m_E9oE25unf ; }
    //! The method to get NHits
    float GetNHits() const { return m_NHits ; }
    //! The method to get Status
    int GetStatus() const { return m_Status ; }
    //! The method to get Grade
    int GetGrade() const { return m_Grade  ; }
    //! The method to get return m_UncEnergy
    float GetUncEnergy() const { return m_UncEnergy ; }
    //! The method to get return m_Time
    float GetTime() const { return m_Time ; }

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



    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLShower() {;}

    ClassDef(ECLShower, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
