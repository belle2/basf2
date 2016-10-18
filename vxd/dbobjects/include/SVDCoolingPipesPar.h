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
  * The Class for VXD Cooling Pipe
  */

  class SVDCoolingPipesLayerPar: public TObject {

  public:

    //! Default constructor
    SVDCoolingPipesLayerPar() {}
    //! Constructor using Gearbox
    explicit SVDCoolingPipesLayerPar(const GearDir& content) { read(content); }
    //! Destructor
    ~SVDCoolingPipesLayerPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:
    int m_nPipes;
    double m_startPhi;
    double m_deltaPhi;
    double m_radius;
    double m_zstart;
    double m_zend;

    ClassDef(SVDCoolingPipesLayerPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for VXD Cooling Pipe
  */

  class SVDCoolingPipesPar: public TObject {

  public:

    //! Default constructor
    SVDCoolingPipesPar() {}
    //! Constructor using Gearbox
    explicit SVDCoolingPipesPar(const GearDir& content) { read(content); }
    //! Destructor
    ~SVDCoolingPipesPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:
    std::string m_material;
    double m_outerDiameter;
    double m_wallThickness;

    std::vector<SVDCoolingPipesLayerPar> m_layers;

    ClassDef(SVDCoolingPipesPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


} // end of namespace Belle2

