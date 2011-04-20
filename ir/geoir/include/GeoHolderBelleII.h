/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hiroshi Nakano                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ############################################################
// for your information, please see BelleII computing homepage and
// see /~twiki/bin/view/Computing/InteractionRegionPackages
// (basf2 Software Portal >> Interaction Region(IR) >> HOLDERgeom.pdf)
// ############################################################

#include <geometry/geodetector/CreatorBase.h>

#include <TGeoShape.h>
#include <TGeoMedium.h>
#include <TGeoMatrix.h>

#include <string>
#include <vector>

#ifndef GEOHOLDERBELLEII_H_
#define GEOHOLDERBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!  The GeoHolderBelleII class.
  /*!
     The creator for the the Belle II Holder.
  */
  class GeoHolderBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoHolderBelleII class.
    GeoHolderBelleII();

    //! The destructor of the GeoHolderBelleII class.
    virtual ~GeoHolderBelleII();

    //! Creates the ROOT Objects for the Holder geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);

  protected:

  private:

  };

}

#endif /* GEOHOLDERBELLEII_H_ */
