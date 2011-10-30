/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DSPECL_H
#define DSPECL_H

//#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class DspECL : public TObject {
  public:


    //! The cell id of this hit.
    int m_eventId;

    //! The cell id of this hit.
    int m_cellId;

    //!  Fit Dsp Array  of this hit.
    int m_DspA[16];


    //! The method to set event id
    void setEventId(int eventId) { m_eventId = eventId; }

    //! The method to set cell id
    void setCellId(int cellId) { m_cellId = cellId; }

    //! The method to set  m_DspArray

    void setDspA(int  DspArray[16]) { for (int i = 0; i < 16; i++) { m_DspA[i] = DspArray[i];} }



    //! The method to get event id
    int getEventId() const { return m_eventId; }

    //! The method to get cell id
    int getCellId() const { return m_cellId; }


    //! The method to getDspArray
//    int getDspA() const { return m_DspA;}




    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    DspECL() {;}

    //! Useful Constructor
    DspECL(
      int eventId,
      int cellId,
      int DspArray[16]
    ) {
      m_eventId = eventId;
      m_cellId = cellId;
//      m_DspA= DspA;
      for (int i = 0; i < 16; i++) { m_DspA[i] = DspArray[i];}
    }

    ClassDef(DspECL, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
