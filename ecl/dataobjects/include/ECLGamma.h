/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLGAMMA_H
#define ECLGAMMA_H

#include <framework/datastore/DataStore.h>
#include "ecl/dataobjects/ECLShower.h"
#include <math.h>


#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class ECLGamma : public TObject {
  public:

    /**shower id of this Gamma. */
    int m_showerId;

    //! The method to set showerId
    void setShowerId(int showerId) { m_showerId = showerId; }

    //! The method to get showerId
    int GetShowerId() const { return m_showerId ; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLGamma() {;}
    /** ROOT Macro.*/
    ClassDef(ECLGamma, 1);

  };

} // end namespace Belle2

#endif
