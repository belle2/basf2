/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Clement Ng, Andreas Moll                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorBase.h>

#include <TGeoShape.h>
#include <TGeoMedium.h>
#include <TGeoMatrix.h>

#include <string>
#include <vector>

#ifndef GeoIRBelleIISymm_H_
#define GeoIRBelleIISymm_H_

namespace Belle2 {

  class GearDir;

  /**
   * The GeoIRBelleIISymm class.
   *
   * The creator for the outer IR geometry of the Belle II detector.
   */
  class GeoIRBelleIISymm : public CreatorBase {

  public:

    /** Constructor of the GeoIRBelleIISymm class. */
    GeoIRBelleIISymm();

    /** The destructor of the GeoIRBelleIISymm class. */
    virtual ~GeoIRBelleIISymm();

    /**
     * Creates the ROOT Objects for the beampipe geometry.
     *
     * @param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
     */
    virtual void create(GearDir& content);

    /**
     * TGeoShape function for BelleII Pipe segment.
     *
     * Will create a pipe shape, optionally cut along x-axis with either:
     *  - different angle ends
     *  - different radius ends
     * but NOT both.
     *
     * @param name_ Name of the pipe shape.
     * @param length_ Halflength of pipe.
     * @param radinn1_ Inner radius at start.
     * @param radout1_ Outer radius at start.
     * @param radchange_ Change in radius.
     * @param theta1_ Angle to previous pipe.
     * @param theta2_ Angle to next pipe.
     * @param cutheight_ X-axis cut distance from centre.
     * @param cutdirection_ X-axis cut direction (0 ignores cut).
     * @param rotation_ Rotation around y-axis.
     */
    TGeoShape* GeoBelleIICutPipe(const char* name_,
                                 const double length_,
                                 const double radinn1_,
                                 const double radout1_,
                                 const double radchange_,
                                 const double theta1_,
                                 const double theta2_,
                                 const double cutheight_,
                                 const int cutdirection_,
                                 const double rotation_);

    /**
     * Pipe stream creator.
     *
     * Fills containers with specified range of pipe segments from GearDir content.
     * Better to use shared_ptr?
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
    void createPipe(const std::string& name_,
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

#endif /* GeoIRBelleIISymm_H_ */
