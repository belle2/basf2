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


namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD geometry
  */

  class SVDGeometryPar: public TObject {

  public:

    //! Default constructor
    SVDGeometryPar() {}

    //! Constructor using Gearbox
    explicit SVDGeometryPar(const GearDir& content) { read(content); }

    //! Destructor
    ~SVDGeometryPar() {}

    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:

    ClassDef(SVDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

