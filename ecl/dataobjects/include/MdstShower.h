/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MDSTSHOWER_H
#define MDSTSHOWER_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class MdstShower : public TObject {
  public:
    /**shower id of this shower. */
    int m_showerId;
    /**Energy of this shower. */
    double m_Energy;
    /**Theta of this shower. */
    double m_Theta;
    /**Phi of this shower. */
    double m_Phi;
    /**Distance of this shower. */
    double m_R;
    /**Error of this shower. */
    double m_Error[6];
    /**Mass of this shower. */
    double m_Mass;
    /**Width of this shower. */
    double m_Width;
    /**E9oE25 of this shower. */
    double m_E9oE25;
    /**E9oE25unf of this shower. */
    double m_E9oE25unf;
    /**NHits of this shower. */
    double m_NHits;
    /**Status of this shower. */
    int m_Status;
    /**Grade of this shower. */
    int m_Grade;
    /**EUncEnergy of this shower. */
    double m_UncEnergy;
    /**Time of this shower. */
    double m_Time;


    //! The method to set showerId
    void setShowerId(int showerId) { m_showerId = showerId; }
    //! The method to set Energy
    void setEnergy(double Energy) { m_Energy = Energy; }
    //! The method to set Theta
    void setTheta(double Theta) { m_Theta = Theta; }
    //! The method to set Phi
    void setPhi(double Phi) { m_Phi = Phi; }
    //! The method to set Distance
    void setR(double R) { m_R = R; }
    //! The method to set Error
    void setError(double  ErrorArray[6]) { for (int i = 0; i < 6; i++) { m_Error[i] = ErrorArray[i];} }
    //! The method to set Mass
    void setMass(double Mass) { m_Mass = Mass; }
    //! The method to set Width
    void setWidth(double Width) { m_Width = Width; }
    //! The method to set E9oE25
    void setE9oE25(double E9oE25) { m_E9oE25 = E9oE25; }
    //! The method to set E9oE25unf
    void setE9oE25unf(double E9oE25unf) { m_E9oE25unf = E9oE25unf; }
    //! The method to set NHits
    void setNHits(double NHits) { m_NHits = NHits; }
    //! The method to set Status
    void setStatus(int Status) { m_Status = Status; }
    //! The method to set Grade
    void setGrade(int Grade) { m_Grade = Grade ; }
    //! The method to set m_UncEnergy
    void setUncEnergy(double UncEnergy) { m_UncEnergy = UncEnergy; }
    //! The method to set m_Time
    void setTime(double Time) { m_Time = Time; }

    //! The method to get showerId
    int GetShowerId() const { return m_showerId ; }
    //! The method to get Energy
    double GetEnergy() const { return m_Energy ; }
    //! The method to get Theta
    double GetTheta() const { return m_Theta ; }
    //! The method to get Phi
    double GetPhi() const { return m_Phi ; }
    //! The method to get Distance
    double GetR() const { return m_R ; }
    //! The method to get Error
    void GetError(double  ErrorArray[6]) { for (int i = 0; i < 6; i++) { ErrorArray[i] = m_Error[i] ;} }
    //! The method to get Mass
    double GetMass() const { return m_Mass ; }
    //! The method to get Width
    double GetWidth() const { return m_Width ; }
    //! The method to get E9oE25
    double GetE9oE25() const { return m_E9oE25 ; }
    //! The method to get E9oE25unf
    double GetE9oE25unf() const { return m_E9oE25unf ; }
    //! The method to get NHits
    double GetNHits() const { return m_NHits ; }
    //! The method to get Status
    int GetStatus() const { return m_Status ; }
    //! The method to get Grade
    int GetGrade() const { return m_Grade  ; }
    //! The method to get return m_UncEnergy
    double GetUncEnergy() const { return m_UncEnergy ; }
    //! The method to get return m_Time
    double GetTime() const { return m_Time ; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    MdstShower() {;}

    ClassDef(MdstShower, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
