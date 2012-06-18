/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                               *
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
    int m_cellId;

    //!  Fit Dsp Array  of this hit.
    int m_DspA[31];

    //! The method to set cell id
    void setCellId(int cellId) { m_cellId = cellId; }

    //! The method to set  m_DspArray

    void setDspA(int  DspArray[31]) { for (int i = 0; i < 31; i++) { m_DspA[i] = DspArray[i];} }


    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! The method to getDspArray
    void getDspA(int  DspArray[31]) const { for (int i = 0; i < 31; i++) {  DspArray[i] = m_DspA[i] ;} }




    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    DspECL() {;}


    ClassDef(DspECL, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
