/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <arich/modules/arichReconstruction/ARICHReconstruction.h>

namespace Belle2 {
  class ARICHHit;
  class Track;
  class ExtHit;
  class ARICHAeroHit;
  class ARICHLikelihood;
  class ARICHTrack;

  /** ARICH subdetector main module
   *
   * This module takes either reconstructed tracks from tracking or GEANT4
   * simulated tracks as input. For every track, the value of ARICH likelihood
   * function is calculated (by calling likelihood2 function), which gives the
   * probability that a track was made by a certain particle. This information
   * is stored in ARICHLikelihood object. Also, number of theoretically
   * expected photons per hypothesis (five in all) and number of detected photons
   * are added to this object.
   * Finally, the module stores the relations between the track candidate,
   * extrapolated track and calculated ARICH likelihood.
   */

  class ARICHReconstructorModule : public Module {

  public:

    /*! Constructor.*/
    ARICHReconstructorModule();

    /*! Destructor.*/
    virtual ~ARICHReconstructorModule();

    /**
     * Initialize the Module.
     *
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     *
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     *
     */
    virtual void event() override;

  protected:

    /*! Print module parameters.*/
    void printModuleParams();

  private:

    StoreArray<ARICHHit> m_ARICHHits;
    StoreArray<Track> m_Tracks;
    StoreArray<ExtHit> m_ExtHits;
    StoreArray<ARICHAeroHit> m_aeroHits;
    StoreArray<ARICHLikelihood> m_ARICHLikelihoods;
    StoreArray<ARICHTrack> m_ARICHTracks;

    // Other members.
    ARICHReconstruction* m_ana;      /**< Class with reconstruction tools */
    double m_trackPositionResolution;/**< Track position resolution; simulation smearing. */
    double m_trackAngleResolution;   /**< Track direction resolution; simulation smearing. */
    int m_inputTrackType;            /**< Input tracks from the tracking (0) or from MCParticles>AeroHits (1). */
    int m_storePhot;              /**< If == 1 individual reconstruced photon information (cherenkov angle,...) is stored in ARICHTrack */
    bool m_align;                    /** If==1 alignment constants are used for global->local track transformation */
    bool m_alignMirrors;             /** If==1 alignment constants for mirrors are used */
  };

} // Belle2 namespace
