/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen, Moritz Gelb, Pablo Goldenzweig,       *
 *               Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FLAVORTAGGINGMODULE_H
#define FLAVORTAGGINGMODULE_H

#include <framework/core/Module.h>

#include <analysis/TMVAInterface/Teacher.h>
#include <analysis/TMVAInterface/Expert.h>

#include <mdst/dataobjects/KLMCluster.h>
#include "mdst/dataobjects/ECLCluster.h"

// track objects
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <string>



namespace Belle2 {

  class Particle;

  /**
   * Flavor Tagging module for modular analysis
   *
   * This module finds the flavour of the non reconstructed B *
   */
  class FlavorTaggingModule : public Module {

  public:

    /**
     * Constructor
     */
    FlavorTaggingModule();

    /**
     * Destructor
     */
    virtual ~FlavorTaggingModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();



  private:


    int m_mode;       /**< Mode of use of the Flavor Tagging Module 0 for Teacher, 1 for Expert*/
    std::string m_listName;               /**< Breco particle list name */
    std::vector<Belle2::Track*> m_tagTracks;        /**< tracks of the rest of the event */
    std::vector<const Belle2::Particle*> m_tagDummies;        /**< Dummy particles belonging to the tracks of the rest of the event */
    std::vector<Belle2::ECLCluster* > m_tagECLClusters;     /**< ECLClusters of the rest of the event */
    //std::vector<Belle2::KLMCluster* > m_tagKLMClusters;     /**< KLMClusters of the rest of the event */

    bool getTagObjects(Particle* Breco);  /** get the tracks, ECLClusters and KLMClusters on the tag side (rest of the event) */
    bool getMC_PDGcodes();      /** get the PDG Codes of the Daughters of B and B-Bar */
    /**
     * Categories
     */
    bool Muon_Category(Particle* Breco);   /** prepare input values for the NN specialized for Muons */
    bool Kaon_Category();   /** prepare input values for the NN specialized for Kaons */
    bool SlowPion_Category();   /** prepare input values for the NN specialized for slow Pions */

    /**
     * Parameters needed for Training
     */
    std::string m_workingDirectory; /**< Working directory in which the config file and the weight file directory is created */
    std::string m_prepareOption; /**< Options which are passed to the TMVA Factory::PrepareTrainingAndTestTree */
    /**
    * Parameters needed for the Muon Category in Teacher Mode in Track Level
    */
    std::string m_methodPrefix_Muon_TL; /**< common prefix for the methods trained by TMVATeacher */
    std::vector<std::string> m_variables_Muon_TL; /**< input variables for the TMVA method */
    std::string m_target_Muon_TL; /**< target used by multivariate analysis method has to be integer valued variable which defines clusters in the sample. */
    std::vector<std::tuple<std::string, std::string, std::string>> m_methods_Muon_TL; /**< tuple(name, type, config) for every method */
    std::string m_factoryOption_Muon_TL; /**< Options which are passed to the TMVA Factory */
    TMVAInterface::Teacher* m_teacher_Muon_TL; /**< Used TMVA method */
    /**
    * Parameters needed for the Muon Category in Expert Mode in Track Level
    */
    std::string m_methodName_Muon_TL; /**< name of the TMVA method specified in the training by TMVATeacher */
    int m_signalCluster_Muon_TL; /**< Number of the cluster which is considered signal.  */
    float m_signalToBackgroundRatio; /**< Signal to background ration to calculate probability, -1 if no transformation of the method output should be performed */
    TMVAInterface::Expert* m_expert_Muon_TL; /**< Method used to calculate the target variable */
    std::string m_signalProbabilityName /**< Name under which the SignalProbability is stored in the ExtraInfo of the Track object. */;
    std::string m_signalRealValueName /**< Name under which the RealValue is stored in the ExtraInfo of the Track object. */;
    int N_Events  /**< Total Number of Events. */;
    int N_noParticle /**< Events where the List Size of the Particle List is 0 (No Breco). */;
    int N_Background /**< Total number of Events where the Target Muon is not in RestofEvent. */;
    int N_noTracks /**< Total number of Events whose tracks could not be fitted. */;
    int N_Target_in_Breco; /**< Total number of Events where the Target Muon is used in Breco. */;
    int N_Signal /**< Total number of Signal Events: The track of the Target Muon in RestofEvent could be recunstructed. */;
    int N_Corr /**< Number of correctly classified Events. */;
    int N_Wrong /**< Number of wrongly classified Events (A wrong Track is classified as Muon). */;
    int N_Wrong_rightFlavor /**< Number of wrongly classified Events but with right flavor (The wrong Track leads to the right flavor). */;
    /**
    * Parameters needed for the Muon Category in Teacher Mode in Event Level
    */
    std::string m_methodPrefix_Muon_EL; /**< common prefix for the methods trained by TMVATeacher */
    std::string m_target_Muon_EL; /**< target used by multivariate analysis method has to be signed integer valued variable which defines clusters in the sample. */
    std::vector<std::string> m_variables_Muon_EL; /**< input variables for the TMVA method */
    std::vector<std::tuple<std::string, std::string, std::string>> m_methods_Muon_EL; /**< tuple(name, type, config) for every method */
    std::string m_factoryOption_Muon_EL; /**< Options which are passed to the TMVA Factory */
    TMVAInterface::Teacher* m_teacher_Muon_EL; /**< Used TMVA method */
    /**
    * Parameters needed for the Kaon Category in Teacher Mode in Track Level
    */
    std::string m_methodPrefix_Kaon_TL; /**< common prefix for the methods trained by TMVATeacher */
    std::vector<std::string> m_variables_Kaon_TL; /**< input variables for the TMVA method */
    std::string m_target_Kaon_TL; /**< target used by multivariate analysis method has to be integer valued variable which defines clusters in the sample. */
    std::vector<std::tuple<std::string, std::string, std::string>> m_methods_Kaon_TL; /**< tuple(name, type, config) for every method */
    std::string m_factoryOption_Kaon_TL; /**< Options which are passed to the TMVA Factory */
    TMVAInterface::Teacher* m_teacher_Kaon_TL; /**< Used TMVA method */
    /**
    * Parameters needed for the Slow Pion Category in Teacher Mode in Track Level
    */
    std::string m_methodPrefix_SlowPion_TL; /**< common prefix for the methods trained by TMVATeacher */
    std::vector<std::string> m_variables_SlowPion_TL; /**< input variables for the TMVA method */
    std::string m_target_SlowPion_TL; /**< target used by multivariate analysis method has to be integer valued variable which defines clusters in the sample. */
    std::vector<std::tuple<std::string, std::string, std::string>> m_methods_SlowPion_TL; /**< tuple(name, type, config) for every method */
    std::string m_factoryOption_SlowPion_TL; /**< Options which are passed to the TMVA Factory */
    TMVAInterface::Teacher* m_teacher_SlowPion_TL; /**< Used TMVA method */
  };
}

#endif /* FLAVORTAGGINGMODULE_H */
