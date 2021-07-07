/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <geometry/CreatorBase.h>
#include <framework/dbobjects/MagneticField.h>

#include <boost/function.hpp>
#include <map>
#include <string>


namespace Belle2 {

  class GearDir;

  /**
   * The GeoMagneticField class.
   *
   * The global creator for the magnetic field of the Belle II detector.
   * It supports homogeneous and radial fields. Special field implementations
   * for subdetectors should be added to the Belle II magnetic field inside the
   * subdetectors' creators.
   */
  class GeoMagneticField : public geometry::CreatorBase {

  public:

    /**
     * Constructor of the GeoMagneticField class.
     */
    GeoMagneticField();

    /**
     * The destructor of the GeoMagneticField class.
     */
    virtual ~GeoMagneticField();

    /**
     * Creates the global ROOT objects and prepares everything for other creators.
     *
     * @param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
     * @param topVolume Geant4 logical top volume.
     * @param type Geometry type.
     */
    virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override;

    /** Nothing to be done when creating from DB, the payload should be found automatically */
    virtual void createFromDB(const std::string&, G4LogicalVolume&, geometry::GeometryTypes) override {}

    virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override;

    /** Create a Database configuration from Gearbox parameters */
    MagneticField createConfiguration(const GearDir& content);

  protected:

    /** Typedef for the map connecting the name of the component to the method reading the parameters.*/
    typedef std::map < std::string, boost::function < void (const GearDir&) > > CompTypeMap;
    CompTypeMap m_componentTypeMap; /**< Maps the name of the component to the function reading the parameters.*/

    /**
     * Reads the parameters for a homogeneous magnetic field and adds the component to the global magnetic field.
     * @param component The GearDir pointing to the component parameter block from which the parameters should be read.
     */
    void readConstantBField(const GearDir& component);

    /**
     * Reads the parameters for a radial magnetic field and adds the component to the global magnetic field.
     * @param component The GearDir pointing to the component parameter block from which the parameters should be read.
     */
    void readRadialBField(const GearDir& component);

    /**
     * Reads the parameters for a quadrupole magnetic field inside beam pipes and adds the component to the global magnetic field.
     * @param component The GearDir pointing to the component parameter block from which the parameters should be read.
     */
    void readQuadBField(const GearDir& component);

    /**
     * Reads the 3D Bfield map and parameters near beam pipes and adds the component to the global magnetic field.
     */
    void readBeamlineBField(const GearDir& component);


    /**
     * Reads the 2D Bfield map and parameters outside of solenoid and adds the component to the global magnetic field.
       Magnetic field map in Belle I are used.
     */

    void readKlm1BField(const GearDir& component);

    /**
     * Reads the parameters for 3d magnetic field (r,phi,z).
     * @param component The GearDir pointing to the component parameter block from which the parameters should be read.
     */
    void read3dBField(const GearDir& component);

    /** Add a constant field component to a magnetic field configuration for the DB */
    void addConstantBField(const GearDir& component, MagneticField& fieldmap);
    /** Add a 3D field component to a magnetic field configuration for the DB */
    void add3dBField(const GearDir& component, MagneticField& fielmap);
  private:

  };

} //end of namespace Belle2
