/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>

#include <framework/database/DBObjPtr.h>

#include <framework/datastore/StoreObjPtr.h>
#include <tracking/dataobjects/RecoTrack.h>


namespace Belle2 {
  /**
   * Computes the track time, defined as the difference between the average of SVD clusters time and the SVDEvent T0
   */
  class trackTimeWriterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    trackTimeWriterModule();

    /** Register input and output data */
    void initialize() ;

    /**  */
    void event() ;


  private:
    /// Name of collection holding the Tracks (input).
    std::string m_trackColName = "";
    /// Name of collection holding the RecoTracks (input).
    std::string m_recoTrackColName = "";
  };
}
