/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//#include <vxd/dataobjects/VxdID.h>
#include <svd/dbobjects/SVDCalibrationsBase.h>
#include <svd/dbobjects/SVDCalibrationsVector.h>
#include <svd/dbobjects/SVDMCFudgeFactorFunction.h>
#include <framework/database/DBObjPtr.h>
//#include <framework/logging/Logger.h>
#include <string>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * SVD position error parameters and provide the position error
   * for the CoGOnly algorithm
   */
  class SVDMCClusterErrorScaleFactor {
  public:
    static std::string name; /**< name of the SVDMCClusterErrorScaleFactor payload */
    typedef SVDCalibrationsBase< SVDCalibrationsVector< SVDMCFudgeFactorFunction > >
    t_payload; /**< typedef for the SVDMCClusterErrorScaleFactor payload of all SVD sensors*/

    /** Constructor, no input argument is required */
    SVDMCClusterErrorScaleFactor() : m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2DEBUG(20, "SVDMCClusterErrorScaleFactor: from now on we are using " <<
                this->m_aDBObjPtr -> get_uniqueID()); });
    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }


  private:

    DBObjPtr< t_payload > m_aDBObjPtr; /**< SVDMCClusterErrorScaleFactor payload */
  };
}
