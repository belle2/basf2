/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_THEANO_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_THEANO_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Expert.h>

/**
 * Python headers include some weird stuff which
 * we don't want to be seen by CLING, otherwise there are
 * a lot of warning during the compiling the dictionary.
 */
#if !defined(__CLING__)
#include <Python.h>
#endif

#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/python/object.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/import.hpp>
#include <boost/python/extract.hpp>
#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif

namespace Belle2 {
  namespace MVA {

    /**
     * Options for the Python MVA method
     */
    class PythonOptions : public SpecificOptions {

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
      virtual std::string getMethod() const override { return "Python"; }

      std::string m_framework = "sklearn"; /**< framework to use e.g. sklearn, xgboost, theano, tensorflow, ... */
      std::string m_steering_file = ""; /**< steering file provided by the user to override the functions in the framework */
      std::string m_config = "null"; /**< Config string in json, which is passed to the get model function */
      unsigned int m_mini_batch_size = 0; /**< Mini batch size, 0 passes the whole data in one call */
      unsigned int m_nIterations = 1; /**< Number of iterations through the whole data */
      double m_training_fraction = 1.0; /**< Fraction of data passed as training data, rest is passed as test data */
      bool m_normalize = false; /**< Normalize the inputs (shift mean to zero and std to 1) */
    };


    /**
     * Teacher for the Python MVA method
     */
    class PythonTeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of this training
       * @param general_options defining all shared options
       * @param specific_options defininf all method specific options
       */
      PythonTeacher(const GeneralOptions& general_options, const PythonOptions& specific_options);

      /**
       * Train a mva method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const override;

    private:
      PythonOptions m_specific_options; /**< Method specific options */
    };

    /**
     * Expert for the Python MVA method
     */
    class PythonExpert : public MVA::Expert {

    public:
      /**
       * Constructs a new Python Expert
       */
      PythonExpert();

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

    protected:
      PythonOptions m_specific_options; /**< Method specific options */
      boost::python::object m_framework; /**< Framework module */
      boost::python::object m_state; /**< current state object of method */
      std::vector<float> m_means; /**< Means of all features for normalization */
      std::vector<float> m_stds; /**< Stds of all features for normalization */
    };

  }
}
#endif
