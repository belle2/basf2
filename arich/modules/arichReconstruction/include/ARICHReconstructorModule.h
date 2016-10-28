/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Dino Tahirovic                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHRECONSTRUCTORMODULE_H
#define ARICHRECONSTRUCTORMODULE_H

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <arich/dataobjects/ARICHTrack.h>
#include <mdst/dataobjects/Track.h>


#include <arich/modules/arichReconstruction/ARICHReconstruction.h>

#include <string>

namespace Belle2 {

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
    virtual void initialize();

    /**
     * Called when entering a new run.
     *
     */
    virtual void beginRun();

    /**
     * Event processor.
     *
     */
    virtual void event();

    /**
     * End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  protected:

    /*! Print module parameters.*/
    void printModuleParams();

  private:

    // Other members.
    ARICHReconstruction* m_ana;      /**< Class with reconstruction tools */
    double m_trackPositionResolution;/**< Track position resolution; simulation smearing. */
    double m_trackAngleResolution;   /**< Track direction resolution; simulation smearing. */
    double m_backgroundLevel;        /**< Photon background level. */
    double m_singleResolution;       /**< Resolution of single photon emission angle. */
    std::vector<double> m_aerogelMerit; /**< Vector of aerogel layer figures of merit. */
    int m_inputTrackType;            /**< Input tracks from the tracking (0) or from MCParticles>AeroHits (1). */
    int    m_storePhot;              /**< If == 1 individual reconstruced photon information (cherenkov angle,...) is stored in ARICHTrack */
  };

} // Belle2 namespace

#endif // ARICHRECONSTRUCTORMODULE
