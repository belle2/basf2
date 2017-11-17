#ifndef CDCTRIGGERNEUROTRAINERMODULE_H
#define CDCTRIGGERNEUROTRAINERMODULE_H

#include <framework/core/Module.h>

#include <trg/cdc/NeuroTrigger.h>
#include <trg/cdc/dataobjects/CDCTriggerMLPData.h>

#include <TH1D.h>

namespace Belle2 {
  /** The trainer module for the neural networks of the CDC trigger.
   * Prepare training data for several neural networks and train them
   * using the Fast Artificial Neural Network library (FANN).
   * For documentation of FANN see http://leenissen.dk/fann/wp/
   */
  class CDCTriggerNeuroTrainerModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    CDCTriggerNeuroTrainerModule();

    /** Destructor. */
    virtual ~CDCTriggerNeuroTrainerModule() {}

    /** Initialize the module.
     * Initialize the networks and register datastore objects.
     */
    virtual void initialize();

    /** Called once for each event.
     * Prepare input and target for each track and store it.
     */
    virtual void event();

    /** Do the training for all sectors. */
    virtual void terminate();

    /** calculate and set the relevant id range for given sector
     *  based on hit counters of the track segments. */
    void updateRelevantID(unsigned isector);

    /** Train a single MLP. */
    void train(unsigned isector);

    /** Save all training samples.
     * @param filename name of the TFile to write to
     * @param arrayname name of the TObjArray holding the training samples in the file
     */
    void saveTraindata(const std::string& filename, const std::string& arrayname = "trainSets");

    /** Load saved training samples.
     * @param filename name of the TFile to read from
     * @param arrayname name of the TObjArray holding the training samples in the file
     * @return true if the training sets were loaded correctly
     */
    bool loadTraindata(const std::string& filename, const std::string& arrayname = "trainSets");

  protected:
    //module parameters
    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;
    /** name of the event time StoreObjPtr */
    std::string m_EventTimeName;
    /** Name of the StoreArray containing the input 2D tracks. */
    std::string m_inputCollectionName;
    /** Switch between MCParticles or RecoTracks as targets. */
    bool m_trainOnRecoTracks;
    /** Name of the MCParticles/RecoTracks collection used as target values. */
    std::string m_targetCollectionName;
    /** Name of file where network weights etc. are stored after training. */
    std::string m_filename;
    /** Name of file where training samples are stored. */
    std::string m_trainFilename;
    /** Name of file where training log is stored. */
    std::string m_logFilename;
    /** Name of the TObjArray holding the networks. */
    std::string m_arrayname;
    /** Name of the TObjArray holding the training samples. */
    std::string m_trainArrayname;
    /** If true, save training curve and parameter distribution of training data. */
    bool m_saveDebug;
    /** Switch to load saved parameters from a previous run. */
    bool m_load;
    /** Parameters for the NeuroTrigger. */
    NeuroTrigger::Parameters m_parameters;
    /** Switch for sector selection during training. */
    bool m_selectSectorByMC;
    /** Number of samples to prepare input ranges. */
    int m_nTrainPrepare;
    /** Input ranges given over the module parameters. */
    std::vector<std::vector<float>> m_IDranges;
    /** Cut on the hit counters to get relevant ID ranges. */
    double m_relevantCut;
    /** Switch to apply cut to single hit counter or to sum over counters. */
    bool m_cutSum;
    /** Minimal number of training samples. */
    double m_nTrainMin;
    /** Maximal number of training samples. */
    double m_nTrainMax;
    /** Switch to multiply number of samples with number of weights. */
    bool m_multiplyNTrain;
    /** Number of validation samples. */
    int m_nValid;
    /** Number of test samples. */
    int m_nTest;
    /** Switch for eary stopping. */
    bool m_stopLoop;
    /** Switch to rescale out of range target values or ignore them. */
    bool m_rescaleTarget;
    /** Limit for weights. */
    double m_wMax;
    /** Number of threads for training. */
    int m_nThreads;
    /** Training is stopped if validation error is higher than
     *  checkInterval epochs ago, i.e. either the validation error is increasing
     *  or the gain is less than the fluctuations. */
    int m_checkInterval;
    /** Maximal number of training epochs. */
    int m_maxEpochs;
    /** Number of training runs with different random start weights. */
    int m_repeatTrain;

    //other data
    /** Instance of the NeuroTrigger. */
    NeuroTrigger m_NeuroTrigger;
    /** Sets of training data for all sectors. */
    std::vector<CDCTriggerMLPData> m_trainSets;
    /** List of input tracks. */
    StoreArray<CDCTriggerTrack> m_tracks;

    //monitoring histograms
    std::vector<TH1D*> phiHistsMC;   /**< phi of MCParticles */
    std::vector<TH1D*> ptHistsMC;    /**< pt of MCParticles */
    std::vector<TH1D*> thetaHistsMC; /**< theta of MCParticles */
    std::vector<TH1D*> zHistsMC;     /**< z of MCParticles */
    std::vector<TH1D*> phiHists2D;   /**< phi of 2D tracks */
    std::vector<TH1D*> ptHists2D;    /**< pt of 2D tracks */
  };
}

#endif
