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


#include <TObject.h>
#include <string>

#include <vxd/dbobjects/VXDGlobalPar.h>

namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD geometry
  */

  class PXDGeometryPar: public TObject {
  public:
    //! Default constructor
    PXDGeometryPar() {}
    //! Constructor using Gearbox
    explicit PXDGeometryPar(const GearDir& content) { read(content); }
    //! Destructor
    ~PXDGeometryPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:

    VXDGlobalPar m_vxdGlobalPar;

    ClassDef(PXDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

