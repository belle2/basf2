/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018, 2021 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <top/utilities/TrackSelector.h>
#include <top/utilities/Chi2MinimumFinder1D.h>
#include <top/reconstruction_cpp/PDFConstructor.h>

#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /**
   * A module for alternative channel T0 calibration with collision data
   * Note: after this kind of calibration one cannot do the geometrical alignment
   * This module can also be used to check the calibration
   */
  class TOPChannelT0CalibratorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPChannelT0CalibratorModule();

    /**
     * Destructor
     */
    virtual ~TOPChannelT0CalibratorModule()
    {}

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };

    // module parameters
    int m_numBins;      /**< number of bins to which search region is divided */
    double m_timeRange; /**< time range in which to search for the minimum [ns] */
    double m_sigmaSmear;  /**< additional smearing of PDF in [ns] */
    std::string m_sample; /**< sample type */
    double m_minMomentum; /**< minimal track momentum if sample is "cosmics" */
    double m_deltaEcms; /**< c.m.s energy window if sample is "dimuon" or "bhabha" */
    double m_dr; /**< cut on POCA in r */
    double m_dz; /**< cut on POCA in z */
    double m_minZ; /**< minimal local z of extrapolated hit */
    double m_maxZ; /**< maximal local z of extrapolated hit */
    std::string m_outFileName; /**< Root output file name containing results */
    std::string m_pdfOption;   /**< PDF option name */

    // procedure
    TOP::TrackSelector m_selector; /**< track selection utility */
    TOP::Chi2MinimumFinder1D m_finders[2][c_numModules][c_numChannels]; /**< finders */
    TOP::PDFConstructor::EPDFOption m_PDFOption = TOP::PDFConstructor::c_Rough; /**< PDF option */

    // datastore objects
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks;    /**< collection of tracks */
    StoreArray<ExtHit> m_extHits;  /**< collection of extrapolated hits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

    // output root file
    TFile* m_file = 0;                 /**< TFile */

    // histograms
    std::vector<TH1F> m_hits1D;  /**< number photon hits in a channel */
    std::vector<TH2F> m_hits2D;  /**< hit times vs. channel */

    // tree and its variables
    TTree* m_tree = 0;  /**< TTree containing selected track parameters etc */
    int m_moduleID = 0; /**< slot to which the track is extrapolated to */
    int m_numPhotons = 0; /**< number of photons in this slot */
    float m_x = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_y = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_z = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_p = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_theta = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_phi = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_pocaR = 0; /**< r of POCA */
    float m_pocaZ = 0; /**< z of POCA */
    float m_pocaX = 0; /**< x of POCA */
    float m_pocaY = 0; /**< y of POCA */
    float m_cmsE = 0; /**< c.m.s. energy if dimuon or bhabha */
    int m_charge = 0; /**< track charge */
    int m_PDG = 0; /**< track MC truth (simulated data only) */

  };

} // Belle2 namespace

