/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

# pragma once
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <top/dataobjects/TOPBackSplashFitResult.h>
#include <top/dataobjects/TOPDigit.h>

#include <RooAbsPdf.h>
#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooFitResult.h>
#include <RooWorkspace.h>

namespace Belle2 {
  /**
   * A module to extract and save the timing of TOP signal in front of neutral
   * hadron ECL clusters (i.e. from hadrons showers where charged particles
   * 'backsplash' from ECL towards TOP)
   */
  class TOPBackSplashTimingModule : public Module {

  public:
    /**
     * Constructor
     */
    TOPBackSplashTimingModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    void initialize() override;

    /**
     * Event processor.
     */
    void event() override;

  private:
    /**
     *  Constructs RooFit objects for fitting
     */
    void prepareFitModels();

    /**
     * Maps azimuthal angle to corresponding TOP slot no.
     * @param angle [radians]
     * @return TOP slot no. (1-16)
     */
    int getModuleFromPhi(double);

    /**
     *  Perform fitting of TOP timing in nearby slot
     *  @param TOP slot no. (1-16) (for labelling)
     *  @param TOP digits in slot
     *  @param cluster energy (for labelling)
     *  @param no. of tracks in same slot as neutral clusters (for labelling)
     *  @param cosTheta of cluster
     *  @return pointer to object containing fit parameters of fit to timing of TOP digits
     */
    TOPBackSplashFitResult* fitTimingDigits(int, std::vector<const TOPDigit*>&, double, double, int);

    /**
     * Creates RooPlots of fitted TOPtiming and save as png
     * @param cluster cosTheta (for labelling)
     * @param cluster energy (for labelling)
     * @param TOP slot no (for labelling)
     * @param RooFit model used to perform fit
     * @param RooRealVar used in fit
     * @param RooFitResult object containing fit parameters
     * @param no. of tracks in same slot as neutral clusters (for labelling)
     */
    void makePlot(double, double, int, RooAbsPdf*, RooRealVar*, RooDataSet, RooFitResult*, int);


    /** plot and save fits of TOP timing */
    bool m_saveFits = false;

    /** minimum energy of ECL clusters to consider [GeV] */
    double m_minClusterE = 0.5;

    /** minimum no. of Cherenkov photons to perform fit */
    int m_minNphotons = 50;

    /** minimum no. of crystals in clusters (non-integer with overlapping clusters) */
    double m_minClusterNHits = 10.0;

    /** whether to fit neutral cluster TOP signal in slots with bkg tracks */
    bool m_includeSlotsWithTracks = false;

    /** saves additional fit parameters (e.g. RooFit errors) to output*/
    bool m_saveMoreFitParams = false;


    /** StoreArray of ECLCluster */
    StoreArray<ECLCluster> m_eclClusters;

    /** StoreArray of TOPBackSplashFitResult */
    StoreArray<TOPBackSplashFitResult> m_fitresult;

    /** StoreArray of TOPDigit */
    StoreArray<TOPDigit> m_digits;

    /** Store Array of Track */
    StoreArray<Track> m_tracks;

    /** container of 11 TOP timing fit params per cosTheta */
    std::array<std::array<double, 11>, 15> m_fitparams;

    /** container of RooWorkSpaces, containing fit funcs per cosTheta */
    std::vector<RooWorkspace> m_wss;
  };

} //end namespace Belle2
