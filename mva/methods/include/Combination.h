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
#ifndef INCLUDE_GUARD_BELLE2_MVA_COMBINATION_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_COMBINATION_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Expert.h>


namespace Belle2 {
  namespace MVA {

    /**
     * Options for the Combination MVA method
     */
    class CombinationOptions : public Options {

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

      std::vector<std::string> m_weightfiles; /**< Weightfiles of all methods we want to combine */

    };

    /**
     * Teacher for the Combination MVA method
     */
    class CombinationTeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of this training
       * @param general_options defining all shared options
       * @param specific_options defininf all method specific options
       */
      CombinationTeacher(const GeneralOptions& general_options, const CombinationOptions& specific_options);

      /**
       * Train a mva method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const override;

    private:
      CombinationOptions m_specific_options; /**< Method specific options */
    };

    /**
     * Expert for the Combination MVA method
     */
    class CombinationExpert : public MVA::Expert {

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
      CombinationOptions m_specific_options;  /**< Method specific options */
      std::vector<std::unique_ptr<Expert>> m_experts; /**< Experts of the methods to combine */
      std::vector<std::vector<std::string>> m_expert_variables; /**< Results of the experts to combine */
    };


  }
}

#endif
