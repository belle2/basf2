/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include "TObject.h"
#include "TString.h"

namespace Belle2 {
  /**
   * This class store the reconstruction configuration of SVDTimeGrouping module
   */

  class SVDTimeGroupingConfiguration: public TObject {
  public:
    /**
    * Default constructor
    */
    SVDTimeGroupingConfiguration(const TString& uniqueID = "")
      : m_uniqueID(uniqueID)
    {};

    /**
     * Get the unique ID  of the calibration
     */
    TString get_uniqueID() const {return m_uniqueID;}

    /** name of the payload */
    static std::string name;

  private:

    /** unique identifier of the SVD reconstruction configuration payload */
    TString m_uniqueID;

    ClassDef(SVDTimeGroupingConfiguration, 1); /**< needed by root*/

  };

}
