#ifndef NEUROTRIGGER_H
#define NEUROTRIGGER_H

#include <trg/cdc/dataobjects/CDCTriggerMLP.h>

namespace Belle2 {

  class CDCTriggerSegmentHit;
  class CDCTriggerTrack;
  class MCParticle;

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
       * The ranges are also valid if nPhi * nPt * nTheta = nMLPs.
       */
      unsigned nMLP = 1;
      /** Number of input nodes. */
      std::vector<int> nInput = {27};
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
      /** Super layer pattern for which MLP is trained for all networks. */
      std::vector<unsigned int> SLpattern = {0};
      /** Maximal drift time, identical for all networks. */
      unsigned tMax = 512;
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

    /** return reference to a neural network */
    CDCTriggerMLP& operator[](unsigned index) { return m_MLPs[index]; }
    /** return const reference to a neural network */
    const CDCTriggerMLP& operator[](unsigned index) const { return m_MLPs[index]; }

    /** return number of neural networks */
    unsigned nSectors() const { return m_MLPs.size(); }

    /** add an MLP to the list of networks */
    void addMLP(CDCTriggerMLP newMLP) { m_MLPs.push_back(newMLP); }

    /** Select one expert MLP based on the track parameters of the given track.
     * This function assumes that sectors are unique.
     * The first matching sector is returned without checking the rest.
     * @return index of the selected MLP, -1 if the track does not fit any sector
     */
    int selectMLP(const CDCTriggerTrack& track);

    /** Select all matching expert MLPs based on the track parameters
     * of the given track estimate or MC particle.
     * This function is used only during training to train overlapping sectors.
     * At the end of the training, sectors are redefined to be unique.
     * @return indices of the selected MLPs, empty if the track does not fit any sector
     */
    std::vector<int> selectMLPs(const CDCTriggerTrack& track,
                                const MCParticle& mcparticle, bool selectByMC);

    /** Calculate 2D phi position and arclength for the given track and store them. */
    void updateTrack(const CDCTriggerTrack& track);

    /** Calculate phi position of a hit relative to 2D track
     * (scaled to number of wires). */
    double getRelId(const CDCTriggerSegmentHit& hit);

    /** Calculate input pattern for MLP.
     * @param isector index of the MLP that will use the input
     * @return super layer pattern of hits in the current track
     */
    unsigned short getInputPattern(unsigned isector);

    /** Calculate input values for MLP.
     * @param isector index of the MLP that will use the input
     * @return scaled vector of input values (1 for each input node)
     */
    std::vector<float> getInputVector(unsigned isector);

    /** Get list of indices for the hits that are selected as input for the MLP. */
    std::vector<unsigned> getSelectedHitIds() { return m_selectedHitIds; }

    /** Run an expert MLP.
     * @param isector index of the MLP
     * @param input vector of input values
     * @return unscaled output values (z vertex in cm and/or theta in radian) */
    std::vector<float> runMLP(unsigned isector, std::vector<float> input);

  private:
    /** List of networks */
    std::vector<CDCTriggerMLP> m_MLPs = {};
    /** Radius of the CDC layers with priority wires (2 per super layer) */
    double m_radius[9][2];
    /** Number of track segments up to super layer */
    unsigned m_TSoffset[10];
    /** 2D phi position of current track scaled to number of wires */
    double m_idRef[9][2];
    /** 2D arclength of current track */
    double m_arclength[9][2];
    /** List of indices for the hits that are selected as input for the MLP */
    std::vector<unsigned> m_selectedHitIds = {};
  };
}
#endif
