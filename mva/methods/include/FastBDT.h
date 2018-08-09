/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_FASTBDT_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_FASTBDT_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Expert.h>

#include <FastBDT.h>

#if FastBDT_VERSION_MAJOR >= 3
#include <FastBDT_IO.h>
#else
#include <IO.h>
#endif

#if FastBDT_VERSION_MAJOR >= 5
#include <Classifier.h>
#endif

// Template specialization to fix NAN sort bug of FastBDT in upto Version 3.2
#if FastBDT_VERSION_MAJOR <= 3 && FastBDT_VERSION_MINOR <= 2
namespace FastBDT {
  template<>
  bool compareIncludingNaN(float i, float j);
}
#endif

namespace Belle2 {
  namespace MVA {


    /**
     * checks if the vector actually contains two classes
     */
    bool isValidSignal(const std::vector<bool>& Signals);

    /**
     * Options for the FANN MVA method
     */
    class FastBDTOptions : public SpecificOptions {

    public:
      /**
       * Load mechanism to load Options from a xml tree
       * @param pt xml tree
       */
      virtual void load(const boost::property_tree::ptree& pt) override;

      /**
       * Save mechanism to store Options in a xml tree
       * @param pt xml tree
       */
      virtual void save(boost::property_tree::ptree& pt) const override;

      /**
       * Returns a program options description for all available options
       */
      virtual po::options_description getDescription() override;

      /**
       * Return method name
       */
      virtual std::string getMethod() const override { return "FastBDT"; }

      unsigned int m_nTrees = 200; /**< Number of trees */
      unsigned int m_nCuts = 8; /**< Number of cut Levels = log_2(Number of Cuts) */
      unsigned int m_nLevels = 3; /**< Depth of tree */
      double m_shrinkage = 0.1; /**< Shrinkage during the boosting step */
      double m_randRatio = 0.5; /**< Fraction of data to use in the stochastic training */
#if FastBDT_VERSION_MAJOR >= 5
      std::vector<unsigned int>
      m_individual_nCuts; /**< Number of cut Levels = log_2(Number of Cuts) for each provided feature. If empty m_nCuts is used for all features*/
      double m_flatnessLoss = -1.0; /**< Flatness Loss constant */
      bool m_sPlot = false; /**< Activates sPlot sampling */
      bool m_purityTransformation = false; /**< Activates purity transformation globally for all features */
      std::vector<bool>
      m_individualPurityTransformation; /**< Vector which decided for each feature individually if the purity transformation should be used. */
#endif
    };


    /**
     * Teacher for the FastBDT MVA method
     */
    class FastBDTTeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of this training
       * @param general_options defining all shared options
       * @param specific_options defininf all method specific options
       */
      FastBDTTeacher(const GeneralOptions& general_options, const FastBDTOptions& specific_options);

      /**
       * Train a mva method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const override;

    private:
      FastBDTOptions m_specific_options; /**< Method specific options */
    };


    /**
     * Expert for the FastBDT MVA method
     */
    class FastBDTExpert : public MVA::Expert {

    public:
      /**
       * Load the expert from a Weightfile
       * @param weightfile containing all information necessary to build the expert
       */
      virtual void load(Weightfile& weightfile) override;

      /**
       * Apply this expert onto a dataset
       * @param test_data dataset
       */
      virtual std::vector<float> apply(Dataset& test_data) const override;

    private:
      FastBDTOptions m_specific_options; /**< Method specific options */
#if FastBDT_VERSION_MAJOR >= 3
#if FastBDT_VERSION_MAJOR >= 5
      bool m_use_simplified_interface = false; /**< Use the simplified FastBDT interface of version 4 */
      FastBDT::Classifier m_classifier; /**< Simplified FastBDT interface: classifier combines preprocessing and forest */
#endif
      FastBDT::Forest<float> m_expert_forest; /**< Forest Expert -> used in case of no purity transformation. */
#else
      FastBDT::Forest m_expert_forest; /**< Forest Expert */
      std::vector<FastBDT::FeatureBinning<float>> m_expert_feature_binning; /**< Forest feature binning */
#endif
    };

  }
}
#endif
