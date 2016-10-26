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
#include <map>


namespace Belle2 {

  class GearDir;

  /**
  * The Class for SVD Cooling Pipes
  */

  class SVDCoolingPipesPar: public TObject {

  public:

    //! Default constructor
    SVDCoolingPipesPar() {}
    //! Constructor using Gearbox
    explicit SVDCoolingPipesPar(int layer, const GearDir& support) { read(layer, support); }
    //! Destructor
    ~SVDCoolingPipesPar() {}
    //! Get geometry parameters from Gearbox
    void read(int, const GearDir&);

  private:
    std::string m_material;
    double m_outerDiameter;
    double m_wallThickness;

    int m_nPipes;
    double m_startPhi;
    double m_deltaPhi;
    double m_radius;
    double m_zstart;
    double m_zend;

    ClassDef(SVDCoolingPipesPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


} // end of namespace Belle2

