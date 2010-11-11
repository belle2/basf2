/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorBase.h>

#include <TGeoShape.h>
#include <TGeoMedium.h>
#include <TGeoMatrix.h>

#include <string>
#include <vector>

#ifndef GEOIRBelleII_H_
#define GEOIRBelleII_H_

namespace Belle2 {

  class GearDir;

  /**
   * The GeoIRBelleII class.
   *
   * The creator for the outer IR geometry of the Belle II detector.
   */
  class GeoIRBelleII : public CreatorBase {

  public:

    /** Constructor of the GeoIRBelleII class. */
    GeoIRBelleII();

    /** The destructor of the GeoIRBelleII class. */
    virtual ~GeoIRBelleII();

    /**
     * Creates the ROOT Objects for the beampipe geometry.
     *
     * @param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
     */
    virtual void create(GearDir& content);

    /**
     * TGeoShape function for BelleII Pipe segment.
     *
     * Will create a pipe shape with either (but NOT both):
     *  - different angle ends
     *  - different radius ends
     *
     *  @param name_ The name of the Pipe segment
     *  @param length_ Halflength of pipe.
     *  @param radinn1_ Inner radius at start.
     *  @param radout1_ Outer radius at start.
     *  @param radchange_ Change in radius.
     *  @param theta1_ Angle to previous pipe.
     *  @param theta2_ Angle to next pipe.
     */
    TGeoShape* GeoBelleIIPipe(const char* name_,
                              const double length_,
                              const double radinn1_,
                              const double radout1_,
                              const double radchange_,
                              const double theta1_,
                              const double theta2_);

    /**
     * Outer pipes creator.
     *
     * Fills containers with specified range of pipes from GearDir content.
     * Manages crotch pipe shape creation.
     * Returns array of end overlap lengths with IP chamber due to the crotch joints.
     *
     * @param name_ **To be filled by Clement**
     * @param shape_ **To be filled by Clement**
     * @param medium_ **To be filled by Clement**
     * @param trans_ **To be filled by Clement**
     * @param content_ **To be filled by Clement**
     * @param start_ **To be filled by Clement**
     * @param end_ **To be filled by Clement**
     * @param solid_ If true create solid pipes.
     */
    double* createPipe(const char* name_,
                       std::vector<TGeoShape*>& shape_,
                       std::vector<TGeoMedium*>& medium_,
                       std::vector<TGeoCombiTrans*>& trans_,
                       GearDir& content_,
                       const double start_,
                       const double end_,
                       const bool solid_);

  protected:

  private:

  };

}

#endif /* GEOIRBelleII_H_ */
