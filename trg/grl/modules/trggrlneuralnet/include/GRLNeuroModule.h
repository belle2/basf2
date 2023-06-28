/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 ***************************************************************************/
#ifndef GRLNEUROMODULE_H
#define GRLNEUROMODULE_H

#include <framework/core/Module.h>

#include <trg/grl/GRLNeuro.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>
#include <trg/grl/dataobjects/GRLMLPData.h>
#include <framework/database/DBObjPtr.h>

#include <TH1D.h>

namespace Belle2 {
  /** The module for application of the neural networks of the GRL.
   */
  class GRLNeuroModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    GRLNeuroModule();

    /** Destructor. */
    virtual ~GRLNeuroModule() {}

    /** Initialize the module.
     * Initialize the networks and register datastore objects.
     */
    virtual void initialize() override;

    /** Called once for each event.
     * Prepare input and target for each track and store it.
     */
    virtual void event() override;

    virtual void terminate() override;

    /** Load saved training samples.
     * @param filename name of the TFile to read from
     * @param arrayname name of the TObjArray holding the training samples in the file
     * @return true if the training sets were loaded correctly
     */
    bool loadTraindata(const std::string& filename, const std::string& arrayname = "trainSets");

  protected:
    //module parameters
    /** Parameters for the NeuroTrigger. */
    GRLNeuro::Parameters m_parameters;
    /** Name of file where network weights etc. are stored. */
    std::vector<std::string> m_weightFileNames;
    /** Name of file where network bias etc. are stored. */
    std::vector<std::string> m_biasFileNames;
    /** Name of the TObjArray holding the networks. */
    std::string m_arrayname;
    /** Name of the StoreObj containing the input GRL. */
    std::string m_GRLCollectionName;
    /** Instance of the NeuroTrigger. */
    GRLNeuro m_GRLNeuro;
    /** switch to use cdc tracks. */
    bool m_use_cdc;
    /** switch to use trg ecl cluster. */
    bool m_use_ecl;
    /** switch to use trg klm summary. */
    bool m_use_klm;
    /** selection which is signal and which is background */
    int trainMode;
    /** Name of the StoreArray containing the ECL clusters. */
    std::string m_TrgECLClusterName;
    /** Number of nodes in all laysers. */
    std::vector<float> m_nNodes;
    /** cut on MVA to separate signal */
    std::vector<float> m_nn_thres;
    /** save the output histogram */
    bool m_saveHist;
    /** Name of root file to save the histogram */
    std::string m_HistFileName;
    /** name of TRG GRL information object */
    std::string m_TrgGrlInformationName;

    //..ECL look up tables
    //std::vector<float> TCPhiLab;
    //std::vector<float> TCcotThetaLab;
    std::vector<int> TCThetaID;
    std::vector<float> TCPhiCOM;
    std::vector<float> TCThetaCOM;
    std::vector<float> TC1GeV;
    std::vector<TH1D*> h_target;

  };
}

#endif
