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

    /**
     * Constructor.
     */
    ARICHReconstructorModule();

    /**
     * Destructor.
     */
    virtual ~ARICHReconstructorModule();

    /**
     * Initialize the Module.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  protected:

    /**
     * Print module parameters.
     */
    void printModuleParams();

  private:

    /** ARICH hits. */
    StoreArray<ARICHHit> m_ARICHHits;

    /** Tracks. */
    StoreArray<Track> m_Tracks;

    /** Extrapolation hits. */
    StoreArray<ExtHit> m_ExtHits;

    /** Aerogel hits. */
    StoreArray<ARICHAeroHit> m_aeroHits;

    /** Likelihoods. */
    StoreArray<ARICHLikelihood> m_ARICHLikelihoods;

    /** ARICH tracks. */
    StoreArray<ARICHTrack> m_ARICHTracks;

    /* Other members. */

    /** Class with reconstruction tools. */
    ARICHReconstruction* m_ana;

    /** Track position resolution; simulation smearing. */
    double m_trackPositionResolution;

    /** Track direction resolution; simulation smearing. */
    double m_trackAngleResolution;

    /** Input tracks from the tracking (0) or from MCParticles>AeroHits (1). */
    int m_inputTrackType;

    /**
     * If == 1, individual reconstruced photon information
     * (Cherenkov angle,...) is stored in ARICHTrack.
     */
    int m_storePhot;

    /**
     * Whether alignment constants are used for global->local
     * track transformation.
     */
    bool m_align;

    /** Whether alignment constants for mirrors are used. */
    bool m_alignMirrors;

  };

} // Belle2 namespace
