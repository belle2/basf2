#ifndef GRLNEUROTRAINERMODULE_H
#define GRLNEUROTRAINERMODULE_H

#include <framework/core/Module.h>

#include <trg/grl/GRLNeuro.h>
#include <trg/grl/dataobjects/GRLMLPData.h>

#include <TH1D.h>

namespace Belle2 {
  /** The trainer module for the neural networks of the CDC trigger.
   * Prepare training data for several neural networks and train them
   * using the Fast Artificial Neural Network library (FANN).
   * For documentation of FANN see http://leenissen.dk/fann/wp/
   */
  class GRLNeuroTrainerModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    GRLNeuroTrainerModule();

    /** Destructor. */
    virtual ~GRLNeuroTrainerModule() {}

    /** Initialize the module.
     * Initialize the networks and register datastore objects.
     */
    virtual void initialize() override;

    /** Called once for each event.
     * Prepare input and target for each track and store it.
     */
    virtual void event() override;

    /** Do the training for all sectors. */
    virtual void terminate() override;

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
    /** Name of the StoreArray containing the ECL clusters. */
    std::string m_TrgECLClusterName;
    /** Name of the StoreArray containing the input 2D tracks. */
    std::string m_2DfinderCollectionName;
    /** Name of the StoreObj containing the input GRL. */
    std::string m_GRLCollectionName;
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
    GRLNeuro::Parameters m_parameters;
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

    /** Instance of the NeuroTrigger. */
    GRLNeuro m_GRLNeuro;
    /** Sets of training data for all sectors. */
    std::vector<GRLMLPData> m_trainSets;

    //..ECL look up tables
    std::vector<int> TCThetaID;
    std::vector<float> TCPhiLab;
    std::vector<float> TCcotThetaLab;
    std::vector<float> TCPhiCOM;
    std::vector<float> TCThetaCOM;
    std::vector<float> TC1GeV;

    /** Number of CDC sectors. */
    int n_cdc_sector;
    /** Number of ECL sectors. */
    int n_ecl_sector;
    /** Number of Total sectors. */
    int n_sector;

    /** Histograms for monitoring */
    std::vector<TH1D*> h_cdc2d_phi_sig;
    std::vector<TH1D*> h_cdc2d_pt_sig;
    std::vector<TH1D*> h_selE_sig;
    std::vector<TH1D*> h_selPhi_sig;
    std::vector<TH1D*> h_selTheta_sig;
    std::vector<TH1D*> h_result_sig;
    std::vector<TH1D*> h_cdc2d_phi_bg;
    std::vector<TH1D*> h_cdc2d_pt_bg;
    std::vector<TH1D*> h_selE_bg;
    std::vector<TH1D*> h_selPhi_bg;
    std::vector<TH1D*> h_selTheta_bg;
    std::vector<TH1D*> h_result_bg;
    std::vector<TH1D*> h_ncdc_sig;
    std::vector<TH1D*> h_ncdcf_sig;
    std::vector<TH1D*> h_ncdcs_sig;
    std::vector<TH1D*> h_ncdci_sig;
    std::vector<TH1D*> h_necl_sig;
    std::vector<TH1D*> h_ncdc_bg;
    std::vector<TH1D*> h_ncdcf_bg;
    std::vector<TH1D*> h_ncdcs_bg;
    std::vector<TH1D*> h_ncdci_bg;
    std::vector<TH1D*> h_necl_bg;

    double radtodeg;
    std::vector<int> scale_bg;

  };
}

#endif
