/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2019 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: tadeas                                                   *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

/* Belle2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /**
  * Set momentum magnitude for RecoTracks to given value (for runs without magnetic field)
  *
  * Take the momentum direction and update its magnitude to artificial value for all RecoTracks - needed for tracks without magnetic field.
  * By default activated automatically, when zero B-field is detected at the origin

  */
  class SetRecoTrackMomentumModule : public Module {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    SetRecoTrackMomentumModule();

    /** Register input and output data */
    virtual void initialize() override;

    /** Loop over RecoTracks and set the momentum magnitude */
    virtual void event() override;

  private:

    bool m_automatic;  /**< Detect the B-field at origin automatically - disable module if non-zero */
    double m_momentum;  /**< Default momentum magnitude (GeV/c) to set for RecoTracks */
    StoreArray<RecoTrack> m_tracks; /**< The array with RecoTracks to work with */
  };
}
