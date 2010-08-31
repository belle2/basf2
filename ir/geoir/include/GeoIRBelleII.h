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

  //!  The GeoIRBelleII class.
  /*!
     The creator for the outer IR geometry of the Belle II detector.
  */
  class GeoIRBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoIRBelleII class.
    GeoIRBelleII();

    //! The destructor of the GeoIRBelleII class.
    virtual ~GeoIRBelleII();

    //! Creates the ROOT Objects for the beampipe geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);
    TGeoShape* GeoBelleIIPipe(const char* name_,
                              const double length_,                   // halflength of pipe
                              const double radinn1_,                  // inner radius at start
                              const double radout1_,                  // outer radius at start
                              const double radchange_,                // change in radius
                              const double theta1_,                   // angle to previous pipe
                              const double theta2_);                  // angle to next pipe

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
