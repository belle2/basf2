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

#ifndef GEOEKLMBelleII_H_
#define GEOEKLMBelleII_H_

namespace Belle2 {

  class GearDir;

  //   The GeoEKLMBelleII class.
  //   The creator for the outer EKLM geometry of the Belle II detector.

  class GeoEKLMBelleII : public CreatorBase {

  public:

    // Constructor of the GeoEKLMBelleII class.
    GeoEKLMBelleII();

    // The destructor of the GeoEKLMBelleII class.
    virtual ~GeoEKLMBelleII();

    // Creates the ROOT Objects for the EKLM geometry.

    virtual void create(GearDir& content);

  protected:

  private:

  };


}

#endif /* GEOEKLMBelleII_H_ */
