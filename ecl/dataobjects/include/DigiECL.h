/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DIGIECL_H
#define DIGIECL_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class DigiECL : public TObject {
  public:


    //! The cell id of this hit.
    int m_eventId;

    //! The cell id of this hit.
    int m_cellId;

    //! Deposited energy of this hit.
    double m_Amp;

    //!  Fit time of this hit.
    double m_TimeFit;

    //!  Fit Quality of this hit.
    double m_Quality;

    //! The method to set cell id
    void setEventId(int eventId) { m_eventId = eventId; }

    //! The method to set cell id
    void setCellId(int cellId) { m_cellId = cellId; }

    //! The method to set Fit deposited energy
    void setAmp(double Amp) { m_Amp = Amp; }

    //! The method to get Fitt time
    void setTimeFit(double TimeFit) { m_TimeFit = TimeFit; }

    //! The method to get  Fit Quality
    void setQuality(double Quality) { m_Quality = Quality; }


    //! The method to get event id
    int getEventId() const { return m_eventId; }

    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! The method to get Fit deposited energy
    double getAmp() const { return m_Amp; }

    //! The method to get Fit time
    double getTimeFit() const { return m_TimeFit; }

    //! The method to get Fit Quality
    double getQuality() const { return m_Quality; }



    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    DigiECL() {;}

    //! Useful Constructor
    DigiECL(
      int eventId,
      int cellId,
      double Amp,
      double TimeFit,
      double Quality
    ) {
      m_eventId = eventId;
      m_cellId = cellId;
      m_Amp = Amp;
      m_TimeFit = TimeFit;
      m_Quality = Quality;
    }

    ClassDef(DigiECL, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
