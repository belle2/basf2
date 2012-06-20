/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HITASSIGNMENTECL_H
#define HITASSIGNMENTECL_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class HitAssignmentECL : public TObject {
  public:


    //! The cell id of this hit.
    int m_showerId;

    //! The cell id of this hit.
    int m_cellId;

    //! The method to set shower id
    void setShowerId(int showerId) { m_showerId = showerId; }

    //! The method to set cell id
    void setCellId(int cellId) { m_cellId = cellId; }

    //! The method to get shower id
    int getShowerId() const { return m_showerId; }

    //! The method to get cell id
    int getCellId() const { return m_cellId; }


    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    HitAssignmentECL() {;}

    //! Useful Constructor
    HitAssignmentECL(
      int showerId,
      int cellId
    ) {
      m_showerId = showerId;
      m_cellId = cellId;
    }

    ClassDef(HitAssignmentECL, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
