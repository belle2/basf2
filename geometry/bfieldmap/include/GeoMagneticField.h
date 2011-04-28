/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOMAGNETICFIELD_H
#define GEOMAGNETICFIELD_H

#include <geometry/geodetector/CreatorBase.h>

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
  class GeoMagneticField : public CreatorBase {

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
     */
    virtual void create(GearDir& content);


  protected:

    typedef std::map < std::string, boost::function < void (GearDir&) > > CompTypeMap; /**< Typedef for the map connecting the name of the component to the method reading the parameters.*/
    CompTypeMap m_componentTypeMap; /**< Maps the name of the component to the function reading the parameters.*/

    /**
     * Reads the parameters for a homogeneous magnetic field and adds the component to the global magnetic field.
     * @param component The GearDir pointing to the component parameter block from which the parameters should be read.
     */
    void readConstantBField(GearDir& component);


  private:

  };

} //end of namespace Belle2

#endif /* GEOMAGNETICFIELD_H */
