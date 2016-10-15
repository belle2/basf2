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
  * The Class for VXD Radiation Sensor Position parameters
  */
  class VXDRadiationSensorPositionPar: public TObject {
  public:
    //! Default constructor
    VXDRadiationSensorPositionPar() {}
    //! Constructor using Gearbox
    explicit VXDRadiationSensorPositionPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDRadiationSensorPositionPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
  private:
    //! Common z position of the sensor centers in mm
    double m_posZ;
    //! Common radial position of the sensor centers in mm
    double m_radius;
    //! Common angle to the z axis, 0 is parallel to z in deg
    double m_theta;
    //! Individual Id number for all radiation sensors
    std::vector<int> m_ids;
    //! Individual Phi angle of radiation sensor in deq
    std::vector<double> m_phis;

    ClassDef(VXDRadiationSensorPositionPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for VXD Radiation Sensor parameters
  */
  class VXDRadiationSensorPar: public TObject {
  public:
    //! Default constructor
    VXDRadiationSensorPar() {}
    //! Constructor using Gearbox
    explicit VXDRadiationSensorPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDRadiationSensorPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
  private:
    //! For PXD they need to be outside the envelope volume, for SVD they are inside the detector.
    bool m_insideEnvelope;
    //! Dimension of the sensor in mm
    double m_width;
    //! Dimension of the sensor in mm
    double m_length;
    //! Dimension of the sensor in um
    double m_height;
    //! Material name
    std::string m_material;
    //! Position of radiation sensors
    std::vector<VXDRadiationSensorPositionPar> m_sensors;

    ClassDef(VXDRadiationSensorPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2

