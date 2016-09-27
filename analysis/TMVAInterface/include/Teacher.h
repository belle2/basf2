/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/TMVAInterface/Method.h>
#include <analysis/TMVAInterface/Config.h>
#include <analysis/VariableManager/Manager.h>

#include <framework/datastore/StoreObjPtr.h>

#include <vector>
#include <string>
#include <set>

class TFile;
class TTree;


namespace Belle2 {

  class Particle;
  template <class T> class RootMergeable;

  namespace TMVAInterface {

    /**
     * Interface to ROOT TMVA Factory.
     * Takes care of extracting variables, spectators and the target from the given particles.
     * Trains defined TMVA methods.
     * Saves all necessary information to use the Expert into a .config file.
     */
    class Teacher  {

    public:
      /**
       * Creates a new Teacher object
       * @param config configuration object
       * @param useExistingData if correct TFile and TTree exists already exists, use the stored samples for training
       */
      Teacher(const TeacherConfig& config, bool useExistingData = false);

      /**
       * Destructor
       */
      ~Teacher();

      /**
       * Disallow copy
       */
      Teacher(const Teacher&) = delete;

      /**
       * Disallow assign
       */
      Teacher& operator=(const Teacher&) = delete;

      /**
       * Adds a training sample. The necessary variables are calculated from the provided particle
       * @param particle Particle which serves as training sample, target variable must be available for this particle.
       * @param weight Additional weight for this sample
       */
      void addSample(const Particle* particle, float weight = 1);

      /**
       * Add a variable `branchName` for all samples that are already in the tree.
       * @param branchName Name of the variable the
       * @param values Float values that should be assigned to the samples. Have to be ordered like the samples that reside in the tree. If the Teacher was invoked with `useExistingData`, remember that there are entries in the tree before the ones that are added with `addSample` or `addClassSample`.
       */
      void addVariable(const std::string& branchName, const std::vector<float>& values);

      /**
       * Get branch in tree as vector
       */
      std::vector<float> getVariable(const std::string& branchName);

      /**
       * Get row in tree as vector
       */
      std::vector<float> getRow(unsigned int index);


      /**
       * Returns distinct integer values in a branch
       */
      std::set<int> getDistinctIntegerValues(const std::string& branchName);

      /**
       * Reduced basket and cache sizes of TTree, if one uses a lot of TMVATeacher modules
       * in a path the memory consumption of O(10MB) of a TTree can be too much to run the path on a fast low memory queue.
       * This method helps to reduce the memory print.
       */
      void enableLowMemoryProfile();

      /**
       * Writes tree to a file
       */
      void writeTree();

      /**
       * Train, test and evaluate all methods
       * @param factoryOption options which are passed to the TMVA::Factory constructor, in most cases default options should be fine.
       * @param prepareOption options which are passed to the TMVA::Factory::PrepareTrainingAndTestTree, in most cases default options should be fine.
       * @param target target used for training, if empty sPlot mode is used!
       * @param weight additional weight variable (multiplied with original weight of each sample)
       * @param maxEventsPerClass maximum number of events given to TMVA per class. TMVA internally uses a vector instead of a tree and therefore looses out-of-core ability.
       * @param splot use every sample twice assuming weight's are splot weights regardless of the assigned classid
       */
      void trainClassification(const std::string& factoryOption = "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification",
                               const std::string& prepareOption = "SplitMode=random:!V",
                               const std::string& target = "",
                               const std::string& weight = "");

      /**
       * Train, test and evaluate all methods using sPlot
       */
      void trainSPlot(const std::string& modelFileName,
                      std::vector<std::string> discriminatingVariables,
                      const std::string& weight = "");
    private:

      /**
       * Internal train method used by both, SPlot and Classification Training
       * @param factoryOption
       * @param prepareOption
       * @param target
       * @param signal_weight
       * @param background_weight
       */
      void train(const std::string& factoryOption, const std::string& prepareOption, const std::string& target,
                 const std::string& signal_weight,
                 const std::string& background_weight);

      /**
       * Set Branch Addresses of m_input
       */
      void setBranchAddresses();

      /**
       * Get class id for splot signal
       */
      int getSPlotClass() const { return splot_class; }

      /**
       * Set class id for splot signal
       */
      void setSPlotClass(int _splot_class) { splot_class = _splot_class; }

      /**
       * Returns Tree with all samples with the given classID
       * @param classID
       */
      TTree* getClassTree(const std::string& target, int classID);

      /**
       * Train a class against the rest and return ptree with the configuration of the resulting training.
       */
      void trainClass(std::map<int, TTree*> class_trees, std::string factoryOption, std::string prepareOption,
                      std::string signal_weight, std::string background_weight, int signalClass);

      double sumOfFormula(std::string formula, TTree* tree) const;

    private:
      TeacherConfig m_config; /**< configuration object of the MVA Interface */

      TFile* m_file; /**< stores TTree and training histograms created by TMVA */
      StoreObjPtr<RootMergeable<TTree>> m_tree; /**< holds training and test signal samples */

      std::vector<const Variable::Manager::Var*> m_variables; /**< Pointers to the input variables loaded from VariableManager */
      std::vector<const Variable::Manager::Var*> m_spectators; /**< Pointers to the input spectators loaded from VariableManager */

      float m_original_weight; /**< Storage for the original weight of the sample */
      std::vector<float> m_input; /**< Storage for input variables */

      int splot_class; /**< Class id for splot signal */

    };

  }
}

