/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_REWEIGHTER_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_REWEIGHTER_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Expert.h>


namespace Belle2 {
  namespace MVA {

    /**
     * Options for the Reweighter MVA method
     */
    class ReweighterOptions : public SpecificOptions {

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
      virtual std::string getMethod() const override { return "Reweighter"; }

      std::string m_weightfile; /**< Weightfile of the reweighting expert */
      std::string m_variable; /**< Variable which decides if the reweighter is applied or not */

    };

    /**
     * Teacher for the Reweighter MVA method
     */
    class ReweighterTeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of this training
       * @param general_options defining all shared options
       * @param specific_options defininf all method specific options
       */
      ReweighterTeacher(const GeneralOptions& general_options, const ReweighterOptions& specific_options);

      /**
       * Train a mva method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const override;

    private:
      ReweighterOptions m_specific_options; /**< Method specific options */
    };

    /**
     * Expert for the Reweighter MVA method
     */
    class ReweighterExpert : public MVA::Expert {

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
      ReweighterOptions m_specific_options;  /**< Method specific options */
      GeneralOptions m_expert_options;  /**< Method general options of the expert */
      std::unique_ptr<Expert> m_expert; /**< Experts used to reweight */
      double m_norm = 0.0; /**< Norm for the weights */
    };


  }
}

#endif
