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
#ifndef INCLUDE_GUARD_BELLE2_MVA_NEUROBAYES_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_NEUROBAYES_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Expert.h>

/**
 * Neurobayes headers include some weird stuff which
 * we don't want to be seen by CLING, otherwise there are
 * a lot of warning during the compiling the dictionary.
 */
#if !defined(__CLING__)
#include <neurobayes/NeuroBayesTeacher.hh> //NeuroBayes Header
#include <neurobayes/NeuroBayesExpert.hh>  //NeuroBayes Header
#endif

//#define DEACTIVATE_NEUROBAYES

#include <memory>

namespace Belle2 {
  namespace MVA {

    /**
     * Check if NeuroBayes is available
     */
    bool IsNeuroBayesAvailable();

    /**
     * Options for the NeuroBayes MVA method
     */
    class NeuroBayesOptions : public SpecificOptions {

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
      virtual std::string getMethod() const override { return "NeuroBayes"; }

      std::string m_lossFunction = "ENTROPY"; /**< Loss function */
      std::string m_regularization = "REG"; /**< Used regularization method */
      int m_preprocessing = 112; /**< Global Feature preprocessing flag */
      int m_pruning = 0; /**< Pruning */
      std::string m_shapeTreat = "DIA"; /**< Output shape treatment */
      float m_momentum = 0; /**< Momentum term */
      int m_weightUpdate = 200; /**< Weight update */
      float m_trainTestRatio = 0.5; /**< Train test ratio */
      int m_nIterations = 100; /**< Maximum number of iterations */
      float m_learningSpeed = 1.0; /**< Learning speed */
      float m_limitLearningSpeed = 1.0; /**< Limit of the learning speed */
      std::string m_method = "BFGS"; /**< Training method */
      std::vector<int> m_preproFlagList; /**< Individual preprocessing flags for each feature */
      std::string m_weightfile = "NeuroBayes.nb"; /**< Name of the weightfile */
    };

    /**
     * Teacher for the NeuroBayes MVA method
     */
    class NeuroBayesTeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of this training
       * @param general_options defining all shared options
       * @param specific_options defininf all method specific options
       */
      NeuroBayesTeacher(const GeneralOptions& general_options, const NeuroBayesOptions& specific_options);

      /**
       * Train a mva method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const override;

    private:
      NeuroBayesOptions m_specific_options; /**< Method specific options */
    };

    /**
     * Expert for the NeuroBayes MVA method
     */
    class NeuroBayesExpert : public MVA::Expert {

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
      NeuroBayesOptions m_specific_options; /**< Method specific options */
      /**
       * We exclude the headers of NeuroBayes during the building of the dictionary,
       * so we have to exclude this member as well.
       */
#if !defined(__CLING__)
      std::unique_ptr<::Expert> m_expert; /**< NeuroBayes expert */
#endif

    };

  }
}
#endif
