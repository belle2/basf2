/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGIT_H
#define ECLDIGIT_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class ECLDigit : public TObject {
  public:

    //! The cell id of this hit.
    int m_cellId;

    //! Deposited energy of this hit.
    int m_Amp;

    //!  Fit time of this hit.
    int m_TimeFit;

    //!  Fit Quality of this hit.
    int m_Quality;


    //! The method to set cell id
    void setCellId(int cellId) { m_cellId = cellId; }

    //! The method to set Fit deposited energy
    void setAmp(int Amp) { m_Amp = Amp; }

    //! The method to get Fitt time
    void setTimeFit(int TimeFit) { m_TimeFit = TimeFit; }

    //! The method to get  Fit Quality
    void setQuality(int Quality) { m_Quality = Quality; }


    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! The method to get Fit deposited energy
    int getAmp() const { return m_Amp; }

    //! The method to get Fit time
    int getTimeFit() const { return m_TimeFit; }

    //! The method to get Fit Quality
    int getQuality() const { return m_Quality; }



    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLDigit() {;}

    ClassDef(ECLDigit, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
