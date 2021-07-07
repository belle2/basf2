/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_PDF_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_PDF_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Expert.h>
#include <mva/utility/Binning.h>

namespace Belle2 {
  namespace MVA {


    /**
     * Options for the PDF MVA method
     */
    class PDFOptions : public SpecificOptions {

    public:
      /**
       * Load mechanism (used by Weightfile) to load Options from a xml tree
       * @param pt xml tree
       */
      virtual void load(const boost::property_tree::ptree& pt) override;

      /**
       * Save mechanism (used by Weightfile) to store Options in a xml tree
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
      virtual std::string getMethod() const override { return "PDF"; }

      std::string m_binning = "frequency"; /**< which type of binning is performed e.g. frequency or equidistant */
      std::string m_mode = "probability"; /**< mode which defines the final output e.g. probability */
      unsigned int m_nBins = 100; /**< number of bins used to bin the data */

    };

    /**
     * Teacher for the PDF MVA method
     */
    class PDFTeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and PDFoptions for this training
       * @param general_options defining all shared options
       * @param specific_options defining all options unique to the PDFTeacher
       */
      PDFTeacher(const GeneralOptions& general_options, const PDFOptions& specific_options);

      /**
       * Train PDF method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const override;

    private:
      PDFOptions m_specific_options; /**< Specific options of the PDF method */
    };

    /**
     * Expert for the PDF MVA method
     */
    class PDFExpert : public MVA::Expert {

    public:
      /**
       * Load the PDF expert from a Weightfile
       * @param weightfile containing all information necessary to build the expert
       */
      virtual void load(Weightfile& weightfile) override;

      /**
       * Apply PDF expert onto a dataset
       * @param test_data dataset
       */
      virtual std::vector<float> apply(Dataset& test_data) const override;

    private:
      PDFOptions m_specific_options; /**< Specific options of the PDF method */
      Binning m_binning; /**< used binning */
      std::vector<double> m_value; /**< value returned by expert for each bin */
    };


  }
}

#endif
