/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOCDCBELLEII_H
#define GEOCDCBELLEII_H

#include <TGeoVolume.h>
#include <framework/geodetector/CreatorBase.h>

namespace Belle2 {

  class GearDir;

//!  The GeoCDCBelleII class.
  /*!
     The creator for the CDC geometry of the Belle II detector.
  */

  class GeoCDCBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoCDCBelleII class.
    GeoCDCBelleII();

    //! The destructor of the GeoCDCBelleII class.
    ~GeoCDCBelleII();

    //! Creates the ROOT Objects for the CDC geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);

  protected:

  private:

  };

} // end of Belle2 namespace

#endif /* GEOCDCBELLEII_H */
