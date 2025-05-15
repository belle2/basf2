/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mva/interface/Expert.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Options.h>

#include <onnxruntime/onnxruntime_cxx_api.h>

namespace Belle2 {
  namespace MVA {
    /**
     * Options for the ONNX MVA method.
     */
    class ONNXOptions : public SpecificOptions {

    public:
      /**
       * Load mechanism to load Options from a xml tree
       * @param pt xml tree
       */
      virtual void load(const boost::property_tree::ptree& pt) override {}

      /**
       * Save mechanism to store Options in a xml tree
       * @param pt xml tree
       */
      virtual void save(boost::property_tree::ptree& pt) const override {}

      /**
       * Returns a program options description for all available options
       */
      virtual po::options_description getDescription() override
      {
        po::options_description desc("ONNX options");
        return desc;
      }

      /**
       * Return method name
       */
      virtual std::string getMethod() const override { return "ONNX"; }
    };

    /**
     * Teacher for the ONNX MVA method.
     * Just there to satisfy the interface - doesn't do anything
     */
    class ONNXTeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of
       * this training
       * @param general_options defining all shared options
       * @param specific_options defining all method specific options
       */
      ONNXTeacher(const GeneralOptions& general_options,
                  const ONNXOptions& specific_options) : Teacher(general_options) {}

      /**
       * Train a mva method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const override
      {
        Weightfile w;
        return w;
      }
    };

    /**
     * Expert for the ONNX MVA method
     */
    class ONNXExpert : public Expert {
    public:
      /**
       * Load the expert from a Weightfile
       * @param weightfile containing all information necessary to build the expert
       */
      virtual void load(Weightfile& weightfile) override;

      /**
       * Apply this expert onto a dataset
       * @param testData dataset
       */
      virtual std::vector<float> apply(Dataset& testData) const override;

    private:
      std::unique_ptr<Ort::Session> m_session;
    };
  } // namespace MVA
} // namespace Belle2
