/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include "Math/Vector3D.h"

namespace Belle2 {

  /** Extra info created by the 3D finder of the CDC trigger. */
  class CDCTrigger3DFinderInfo : public RelationsObject {
  public:
    /** default constructor */
    CDCTrigger3DFinderInfo():
      m_houghspace(std::vector<ROOT::Math::XYZVector>()), m_ndreadout(std::vector<ROOT::Math::XYZVector>()) { }

    /** constructor with arguments */
    CDCTrigger3DFinderInfo(std::vector<ROOT::Math::XYZVector>& houghspace, std::vector<ROOT::Math::XYZVector>& ndreadout):
      m_houghspace(houghspace), m_ndreadout(ndreadout) { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTrigger3DFinderInfo() { }

    /** get list of cell member indices */
    std::vector<ROOT::Math::XYZVector> getHoughSpace() const { return m_houghspace; }

    std::vector<ROOT::Math::XYZVector> getNDReadout() const { return m_ndreadout; }

    // setters
    /** set int values */
    void setInfoInts(std::vector<ROOT::Math::XYZVector> houghspace, std::vector<ROOT::Math::XYZVector> ndreadout)
    {
      m_houghspace = houghspace;
      m_ndreadout = ndreadout;
    }

  protected:

    /** list of hough space weights */
    std::vector<ROOT::Math::XYZVector> m_houghspace = {};

    /** list of cell indices of all cluster */
    std::vector<ROOT::Math::XYZVector> m_ndreadout = {};

    //! Needed to make the ROOT object storable
    ClassDef(CDCTrigger3DFinderInfo, 1);
  };
}
