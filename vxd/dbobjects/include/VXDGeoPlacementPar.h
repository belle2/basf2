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
#include <vector>


namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD placement payload
  */
  class VXDGeoPlacementPar: public TObject {
  public:
    //! Default constructor
    VXDGeoPlacementPar() {}
    //! Constructor using Gearbox
    explicit VXDGeoPlacementPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDGeoPlacementPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
  private:
    std::string m_type;
    double m_posU;
    double m_posV;
    double m_posW;
    double m_offsetW;

    ClassDef(VXDGeoPlacementPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

