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
  * The Class for VXD geometry component payload
  */
  class VXDGeoComponentPar: public TObject {

  public:

    //! Default constructor
    VXDGeoComponentPar() {}
    //! Constructor using Gearbox
    explicit VXDGeoComponentPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDGeoComponentPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:
    std::string m_materialName;
    std::string m_colorName;
    double m_width;
    double m_width2;
    double m_length;
    double m_height;
    double m_angle;

    ClassDef(VXDGeoComponentPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };

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





  /*
  VXDEnvelopePar
  ....
  */

  /*
  AlignablePar
  double dU = params.getLength("du") / Unit::mm;
  double dV = params.getLength("dv") / Unit::mm;
  double dW = params.getLength("dw") / Unit::mm;
  double alpha = params.getAngle("alpha");
  double beta  = params.getAngle("beta");
  double gamma = params.getAngle("gamma");
  */


  /*
  VXDGeoSensor sensor(
          paramsSensor.getString("Material"),
          paramsSensor.getString("Color", ""),
          paramsSensor.getLength("width") / Unit::mm,
          paramsSensor.getLength("width2", 0) / Unit::mm,
          paramsSensor.getLength("length") / Unit::mm,
          paramsSensor.getLength("height") / Unit::mm,
          paramsSensor.getBool("@slanted", false)
        );

  sensor.setActive(VXDGeoComponent(
                           paramsSensor.getString("Material"),
                           paramsSensor.getString("Active/Color", "#f00"),
                           paramsSensor.getLength("Active/width") / Unit::mm,
                           paramsSensor.getLength("Active/width2", 0) / Unit::mm,
                           paramsSensor.getLength("Active/length") / Unit::mm,
                           paramsSensor.getLength("Active/height") / Unit::mm
                         ), VXDGeoPlacement(
                           "Active",
                           paramsSensor.getLength("Active/u") / Unit::mm,
                           paramsSensor.getLength("Active/v") / Unit::mm,
                           paramsSensor.getString("Active/w", "center"),
                           paramsSensor.getLength("Active/woffset", 0) / Unit::mm
                         ));
        sensor.setSensorInfo(createSensorInfo(GearDir(paramsSensor, "Active")));
        sensor.setComponents(getSubComponents(paramsSensor));
        m_sensorMap[sensorTypeID] = sensor;

  sensor.setSensorInfo(createSensorInfo(GearDir(paramsSensor, "Active")));
        sensor.setComponents(getSubComponents(paramsSensor));
        m_sensorMap[sensorTypeID] = sensor;
  */



  /*
  VXDGeoLadder(
                   layer,
                   paramsLadder.getLength("shift") / Unit::mm,
                   paramsLadder.getLength("radius") / Unit::mm,
                   paramsLadder.getAngle("slantedAngle", 0),
                   paramsLadder.getLength("slantedRadius", 0) / Unit::mm,
                   paramsLadder.getLength("Glue/oversize", 0) / Unit::mm,
                   paramsLadder.getString("Glue/Material", "")
                 );

  m_ladder.addSensor
  */


  /*
  m_ladder.addSensor(VXDGeoSensorPlacement(
                             sensorInfo.getInt("@id"),
                             sensorInfo.getString("@type"),
                             sensorInfo.getLength(".") / Unit::mm,
                             sensorInfo.getBool("@flipU", false),
                             sensorInfo.getBool("@flipV", false),
                             sensorInfo.getBool("@flipW", false)
                           ));
  */






  /**
  * The Class for VXD geometry
  */

  class VXDGeometryPar: public TObject {

  public:

    //! Default constructor
    VXDGeometryPar() {}

    //! Constructor using Gearbox
    explicit VXDGeometryPar(const GearDir& content) { read(content); }

    //! Destructor
    ~VXDGeometryPar() {}

    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:

    //! tolerance for the energy deposition in electrons to be merged in a single step
    float m_electronTolerance {100};
    //! minimum number of electrons to be deposited by a particle to be saved
    float m_minimumElectrons {10};
    //! Stepsize to be used inside active volumes
    double m_activeStepSize {0.005};
    //! Make also chips sensitive.
    bool m_activeChips {false};
    //! Make sensitive detectors also see neutrons.
    bool m_seeNeutrons {false};
    //! If true only create TrueHits from primary particles and ignore secondaries
    bool m_onlyPrimaryTrueHits {false};
    /** If this is true, only active Materials will be placed for tracking
     * studies. Dead Material will be ignored */
    bool m_onlyActiveMaterial {false};
    //! default material
    std::string m_defaultMaterial;

    ClassDef(VXDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

