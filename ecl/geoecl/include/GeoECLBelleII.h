/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorBase.h>

#include <string>

#ifndef GEOECLBELLEII_H_
#define GEOECLBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!  The GeoECLBelleII class.
  /*!
     The creator for the tutorial geometry of the Belle II detector.
  */
  class GeoECLBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoECLBelleII class.
    GeoECLBelleII();

    //! The destructor of the GeoECLBelleII class.
    virtual ~GeoECLBelleII();

    //! Creates the ROOT Objects for the Tutorial geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);
//    virtual void printtest(GearDir& content);

    /*!
      \create FWendcap crystal
    */
    virtual void makeFWENDCAP(GearDir& content, TGeoVolumeAssembly* group);
    /*!
      \create Barrel  crystal
    */
    virtual void makeBarrel(GearDir& content, TGeoVolumeAssembly* group);
    /*!
      \create BWendcap crystal
    */
    virtual void makeBWENDCAP(GearDir& content, TGeoVolumeAssembly* group);



  protected:

  private:

  };

}

#endif /* GEOECLBELLEII_H_ */
