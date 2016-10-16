/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vxd/dbobjects/VXDPolyConePar.h>
#include <TObject.h>
#include <string>
#include <vector>


namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD support
  */

  class PXDSupportPar: public TObject {

  public:

    //! Default constructor
    PXDSupportPar() {}
    //! Constructor using Gearbox
    explicit PXDSupportPar(const GearDir& content) { read(content); }
    //! Destructor
    ~PXDSupportPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
    //! Get endflanges
    std::vector<VXDPolyConePar> getEndflanges(void) const { return m_endflanges; }

  private:
    //! Endflanges
    std::vector<VXDPolyConePar> m_endflanges;

    ClassDef(PXDSupportPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

