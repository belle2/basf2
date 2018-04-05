/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHINFO_H
#define ARICHINFO_H

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {


  //! Datastore class to keep ARICH event infomation e.g. trigger type.
  class ARICHInfo : public RelationsObject {
  public:

    //! Empty constructor
    /*! Recommended for ROOT IO */
    ARICHInfo():
      m_trgtype(0.0)
    {
      /*! does nothing */
    }

    //! Useful Constructor
    /*!
      \param trgtype obtained from B2L data
    */
    ARICHInfo(int trgtype)
    {
      m_trgtype = trgtype;
    }

    //! Get the trigger type
    int gettrgtype() const { return m_trgtype; }

  private:

    int m_trgtype;            /**< trigger type */


    ClassDef(ARICHInfo, 1); /**< the class title */

  };

} // end namespace Belle2

#endif  // ARICHINFO_H
