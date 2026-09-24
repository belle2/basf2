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
#include <top/dataobjects/TOPRecBunch.h>

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
     * @param phi angle [radians]
     * @return TOP slot no. (1-16)
     */
    static int getModuleFromPhi(double phi);

    /**
     *  Perform fitting of TOP timing in nearby slot
     *  @param moduleID TOP slot no. (1-16) (for labelling)
     *  @param digitsPerSlot TOP digits in slot
     *  @param clusterE ECL cluster nergy (for labelling)
     *  @param clusterCosTheta cosTheta of the ECL cluster
     *  @param nTracksPerSlot no. of tracks in same slot as neutral clusters (for labelling)
     *  @return pointer to object containing fit parameters of fit to timing of TOP digits
     */
    TOPBackSplashFitResult* fitTimingDigits(int moduleID, const std::vector<const TOPDigit*>& digitsPerSlot,
                                            double clusterE, double clusterCosTheta, int nTracksPerSlot);

    /**
     * Creates RooPlots of fitted TOPtiming and save as png
     * @param cosTheta cluster cosTheta (for labelling)
     * @param clusterE cluster energy (for labelling)
     * @param moduleID TOP slot no (for labelling)
     * @param model RooFit model used to perform fit
     * @param x RooRealVar used in fit
     * @param data RooDataSet with the data to be fitted
     * @param res RooFitResult object containing fit parameters
     * @param nTracksPerSlot no. of tracks in same slot as neutral clusters (for labelling)
     */
    void makePlot(double cosTheta, double clusterE, int moduleID, RooAbsPdf* model,
                  RooRealVar* x, RooDataSet data, RooFitResult* res, int nTracksPerSlot);


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

    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

    /** container of 11 TOP timing fit params per cosTheta */
    std::array<std::array<double, 11>, 15> m_fitparams;

    /** container of RooWorkSpaces, containing fit funcs per cosTheta */
    std::vector<RooWorkspace> m_wss;
  };

} //end namespace Belle2
