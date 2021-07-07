/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef NEUROTRIGGER_H
#define NEUROTRIGGER_H

#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
#include <framework/dataobjects/BinnedEventT0.h>

namespace Belle2 {

  class CDCTriggerTrack;

  /** Class to represent the CDC Neurotrigger.
   *
   * The Neurotrigger consists of one or several Multi Layer Perceptrons.
   * The input values are calculated from track segment hits and a 2D track estimate.
   * The output is a scaled estimate of the z-vertex of the track.
   * In case of several MLPs, each is an expert for a different track parameter region.
   *
   * @sa Neurotrigger Modules:
   * @sa NeuroTriggerTrainerModule for preparing training data and training,
   * @sa NeuroTriggerModule for loading trained networks and using them.
   */
  class NeuroTrigger {
  public:
    /** Struct to keep neurotrigger parameters.
     * Contains all information that is needed to initialize several
     * expert MLPs (not including values determined during training).
     */
    struct Parameters {
      /** Number of networks.
       * For network specific parameters you can give either a list with
       * values for each network, or a single value that will be used for all.
       * The ranges are also valid if nPhi * nPt * nTheta * nPattern = nMLPs.
       */
      unsigned nMLP = 1;
      /** Number of nodes in each hidden layer for all networks
       * or factor to multiply with number of inputs.
       * The number of layers is derived from the shape.
       */
      std::vector<std::vector<float>> nHidden = {{3.}};
      /** train z as output */
      bool targetZ = true;
      /** train theta as output */
      bool targetTheta = false;
      /** If true, multiply nHidden with number of input nodes. */
      bool multiplyHidden = true;
      /** Output scale for all networks. */
      std::vector<std::vector<float>> outputScale = {{ -1., 1.}};
      /** Phi region for which MLP is used in degree for all networks. */
      std::vector<std::vector<float>> phiRange = {{0., 360.}};
      /** Charge / Pt region for which MLP is used in 1/GeV for all networks. */
      std::vector<std::vector<float>> invptRange = {{ -5., 5.}};
      /** Theta region for which MLP is used in degree for all networks. */
      std::vector<std::vector<float>> thetaRange = {{17., 150.}};
      /** Phi region for which MLP is trained in degree for all networks.
       *  Can be larger than use range to avoid edge effects. */
      std::vector<std::vector<float>> phiRangeTrain = {{0., 360.}};
      /** Charge / Pt region for which MLP is trained in 1/GeV for all networks.
       *  Can be larger than use range to avoid edge effects. */
      std::vector<std::vector<float>> invptRangeTrain = {{ -5., 5.}};
      /** Theta region for which MLP is trained in degree for all networks.
       *  Can be larger than use range to avoid edge effects. */
      std::vector<std::vector<float>> thetaRangeTrain = {{17., 150.}};
      /** Maximum number of hits in a single super layer for all networks. */
      std::vector<unsigned short> maxHitsPerSL = {1};
      /** Super layer pattern for which MLP is trained for all networks.
       *  Binary pattern of 9 * maxHitsPerSL bits (on/off for each hit).
       *  0 in bit <i>: hits from super layer <i> are not used.
       *  1 in bit <i>: hits from super layer <i> are used.
       *  SLpattern = 0: use any hits present, don't check the pattern. */
      std::vector<unsigned long> SLpattern = {0};
      /** Super layer pattern mask for which MLP is trained for all networks.
       *  Binary pattern of 9 * maxHitsPerSL bits (on/off for each hit).
       *  0 in bit <i>: super layer <i> may or may not have a hit.
       *  1 in bit <i>: super layer <i>
       *                - must have a hit if SLpattern bit <i> = 1
       *                - must not have a hit if SLpattenr bit <i> = 0 */
      std::vector<unsigned long> SLpatternMask = {0};
      /** Maximal drift time, identical for all networks. */
      unsigned tMax = 256;
      /** Determine, how the event time should be obtained. The options are:
      *   "etf_only"                 :   only ETF info is used, otherwise an error
      *                                  is thrown.
      *   "fastestpriority"          :   event time is estimated by fastest priority
      *                                  time in selected track segments. if something
      *                                  fails, it is set to 0.
      *   "zero"                     :   the event time is set to 0.
      *   "etf_or_fastestpriority"   :   the event time is obtained by the ETF, if
      *                                  not possible, the flag
      *                                  "fastestppriority" is used.
      *   "etf_or_zero"              :   the event time is obtained by the ETF, if
      *                                  not possible, it es set to 0
      */
      std::string et_option = "etf_or_fastestpriority";
      /** DEPRECATED!! If true, determine event time from relevant hits if it is missing. */
      bool T0fromHits = false;

    };

    /** Default constructor. */
    NeuroTrigger() {}

    /** Default destructor. */
    virtual ~NeuroTrigger() {}

    /** Set parameters and get some network independent parameters. */
    void initialize(const Parameters& p);

    /** Get indices for sector ranges in parameter lists. */
    std::vector<unsigned> getRangeIndices(const Parameters& p, unsigned isector);

    /** Save MLPs to file.
     * @param filename name of the TFile to write to
     * @param arrayname name of the TObjArray holding the MLPs in the file
     */
    void save(const std::string& filename, const std::string& arrayname = "MLPs");
    /** Load MLPs from file.
     * @param filename name of the TFile to read from
     * @param arrayname name of the TObjArray holding the MLPs in the file
     * @return true if the MLPs were loaded correctly
     */
    bool load(const std::string& filename, const std::string& arrayname = "MLPs");

    /** Loads parameters from the geometry and precalculates some constants
     * that will be needed. */
    void setConstants();

    /** set fixed point precision */
    void setPrecision(const std::vector<unsigned>& precision) { m_precision = precision; }

    /** set the hit collection and event time to required
     * and store the hit collection name */
    void initializeCollections(std::string hitCollectionName, std::string eventTimeName, const std::string& et_option);

    /** return reference to a neural network */
    CDCTriggerMLP& operator[](unsigned index) { return m_MLPs[index]; }
    /** return const reference to a neural network */
    const CDCTriggerMLP& operator[](unsigned index) const { return m_MLPs[index]; }

    /** return number of neural networks */
    unsigned nSectors() const { return m_MLPs.size(); }

    /** add an MLP to the list of networks */
    void addMLP(const CDCTriggerMLP& newMLP) { m_MLPs.push_back(newMLP); }

    /** Select all matching expert MLPs based on the given track parameters.
     * If the sectors are overlapping, there may be more than one matching expert.
     * During training this is intended, afterwards sectors should be redefined to be unique.
     * For unique geometrical sectors, this function can still find several experts
     * with different sector patterns.
     * @return indices of the selected MLPs, empty if the track does not fit any sector
     */
    std::vector<int> selectMLPs(float phi0, float invpt, float theta);

    /** Select one MLP from a list of sector indices.
     * The selected expert either matches the given sector pattern,
     * or has no pattern restriction. An unrestricted expert is returned only
     * if there is no exactly matching expert.
     * @return index of the selected MLP, -1 if no matching MLP is found */
    int selectMLPbyPattern(std::vector<int>& MLPs, unsigned long pattern, const bool neurotrackinputmode);

    /** Calculate 2D phi position and arclength for the given track and store them. */
    void updateTrack(const CDCTriggerTrack& track);

    /** Calculate 2D phi position and arclength for the given track and store them. */
    void updateTrackFix(const CDCTriggerTrack& track);

    /** Calculate phi position of a hit relative to 2D track
     * (scaled to number of wires). */
    double getRelId(const CDCTriggerSegmentHit& hit);
    /** Read out the event time and store it.
     * It can be given different options in the et_option ("EventTime option")
     * parameter.
     * The different options are:
     *   "etf_only"                 :   only ETF info is used, otherwise an error
     *                                  is thrown.
     *   "fastestpriority"          :   event time is estimated by fastest priority
     *                                  time in selected track segments. if something
     *                                  fails, it is set to 0.
     *   "zero"                     :   the event time is set to 0.
     *   "etf_or_fastestpriority"   :   the event time is obtained by the ETF, if
     *                                  not possible, the flag
     *                                  "fastestppriority" is used.
     *   "etf_or_zero"              :   the event time is obtained by the ETF, if
     */
    void getEventTime(unsigned isector, const CDCTriggerTrack& track, std::string et_option, const bool);

    /** DEPRECATED!! Read out the event time and store it.
     * If there is no valid event time, it can be determined
     * from the shortest priority time of all hit candidates,
     * if the option is enabled for the given sector. */
    void getEventTime(unsigned isector, const CDCTriggerTrack& track);

    /** Return value of m_et_option */
    std::string get_et_option()
    {
      std::string eto = m_MLPs[0].get_et_option();
      for (unsigned int i = 0; i < m_MLPs.size(); ++i) {
        if (m_MLPs[i].get_et_option() != eto) {
          B2ERROR("Timing options in the expert networks in the CDC Neurotrigger differ!");
        }
      }
      return eto;

    }

    /** Calculate input pattern for MLP.
     * @param isector index of the MLP that will use the input
     * @param track   axial hit relations are taken from given track
     * @return super layer pattern of hits in the current track
     */
    unsigned long getInputPattern(unsigned isector, const CDCTriggerTrack& track, const bool neurotrackinputmode);
    /** Get complete hit pattern of neurotrack. This does the same as
     * the getInputPattern function, but also shows the axial hit bits.
     * This function was made for the simulation of the hardware debug
     * information "TSVector".
     */
    unsigned long getCompleteHitPattern(unsigned isector, const CDCTriggerTrack& track, const bool neurotrackinputmode);
    /** Get the drift threshold bits, where the time of the TS was outside of the accepted time window and thus
     * shifted to the allowed maximum within the borders. Note, that to get the same values as from the unpacker,
     * this value has to be combined with the (complement of the) TSVector.
     */
    unsigned long getPureDriftThreshold(unsigned isector, const CDCTriggerTrack& track, const bool neurotrackinputmode);

    /** Select hits for each super layer from the ones related to input track
     * @param isector              index of the MLP that will use the input
     * @param track                all hit relations are taken from given track
     * @param returnAllRelevant    if true, return all relevant hits instead of
     *                             selecting the best (for making relations)
     * @return list of selected hit indices */
    std::vector<unsigned> selectHitsHWSim(unsigned isector, const CDCTriggerTrack& track);

    /** Select best hits for each super layer
     * @param isector              index of the MLP that will use the input
     * @param track                axial hit relations are taken from given track
     * @param returnAllRelevant    if true, return all relevant hits instead of
     *                             selecting the best (for making relations)
     * @return list of selected hit indices */
    std::vector<unsigned> selectHits(unsigned isector, const CDCTriggerTrack& track,
                                     bool returnAllRelevant = false);

    /** Calculate input values for MLP.
     * @param isector index of the MLP that will use the input
     * @param hitIds hit indices to be used for the input
     * @return scaled vector of input values (1 for each input node)
     */
    std::vector<float> getInputVector(unsigned isector, const std::vector<unsigned>& hitIds);

    /** Run an expert MLP.
     * @param isector index of the MLP
     * @param input vector of input values
     * @return unscaled output values (z vertex in cm and/or theta in radian) */
    std::vector<float> runMLP(unsigned isector, const std::vector<float>& input);

    /** Run an expert MLP with fixed point arithmetic. */
    std::vector<float> runMLPFix(unsigned isector, std::vector<float> input);

  private:
    /** List of networks */
    std::vector<CDCTriggerMLP> m_MLPs = {};
    /** Radius of the CDC layers with priority wires (2 per super layer) */
    double m_radius[9][2] = {};
    /** Number of track segments up to super layer */
    unsigned m_TSoffset[10] = {};
    /** 2D phi position of current track scaled to number of wires */
    double m_idRef[9][2] = {};
    /** 2D crossing angle of current track */
    double m_alpha[9][2] = {};
    /** Event time of current event / track */
    int m_T0 = 0;
    /** Flag to show if stored event time is valid */
    bool m_hasT0 = false;
    /** Fixed point precision in bit after radix point.
     *  8 values:
     *  - 2D track parameters: omega, phi
     *  - geometrical values derived from track: crossing angle, reference wire ID
     *  - scale factor: radian to wire ID
     *  - MLP values: nodes, weights, activation function LUT input (LUT output = nodes)
     */
    std::vector<unsigned> m_precision;

    /** StoreArray containing the input track segment hits. */
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
    /** StoreObjPtr containing the event time. */
    StoreObjPtr<BinnedEventT0> m_eventTime;
    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;
    /** get NNT payload from database. */
    DBObjPtr<CDCTriggerNeuroConfig> m_cdctriggerneuroconfig;

  };
}
#endif
