/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/PDF.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace MVA {

    void PDFOptions::load(const boost::property_tree::ptree& pt)
    {
      int version = pt.get<int>("PDF_version");
      if (version != 1) {
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }
      m_binning = pt.get<std::string>("PDF_binning");
      m_mode = pt.get<std::string>("PDF_mode");
      m_nBins = pt.get<unsigned int>("PDF_nBins");

    }

    void PDFOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("PDF_version", 1);
      pt.put("PDF_binning", m_binning);
      pt.put("PDF_mode", m_mode);
      pt.put("PDF_nBins", m_nBins);
    }

    po::options_description PDFOptions::getDescription()
    {
      po::options_description description("PDF options");
      description.add_options()
      ("binning", po::value<std::string>(&m_binning), "Binning used to bin the data  (frequency|equidistant)")
      ("nBins", po::value<unsigned int>(&m_nBins), "Number of bins used to bin the data")
      ("mode", po::value<std::string>(&m_mode), "Mode used to calculate return value of expert");
      return description;
    }


    PDFTeacher::PDFTeacher(const GeneralOptions& general_options, const PDFOptions& specific_options) : Teacher(general_options),
      m_specific_options(specific_options) { }

    Weightfile PDFTeacher::train(Dataset& training_data) const
    {

      unsigned int numberOfFeatures = training_data.getNumberOfFeatures();
      if (numberOfFeatures != 1) {
        B2ERROR("PDF method only supports exactly one feature!");
        throw std::runtime_error("PDF method only supports exactly one feature!");
      }
      unsigned int numberOfEvents = training_data.getNumberOfEvents();

      std::vector<float> data(numberOfEvents);
      std::vector<float> weights(numberOfEvents);
      std::vector<bool> isSignal(numberOfEvents);
      for (unsigned int iEvent = 0; iEvent < numberOfEvents; ++iEvent) {
        training_data.loadEvent(iEvent);
        data[iEvent] = training_data.m_input[0];
        weights[iEvent] = training_data.m_weight;
        isSignal[iEvent] = training_data.m_isSignal;
      }

      unsigned int nBins = m_specific_options.m_nBins;

      Binning binning;
      if (m_specific_options.m_binning == "frequency")
        binning = Binning::CreateEqualFrequency(data, weights, isSignal, nBins);
      else if (m_specific_options.m_binning == "equidistant")
        binning = Binning::CreateEquidistant(data, weights, isSignal, nBins);
      else {
        B2ERROR("Unkown binning in PDF Teacher named " << m_specific_options.m_binning);
        throw std::runtime_error("Unkown binning in PDF Teacher named " + m_specific_options.m_binning);
      }

      Weightfile weightfile;
      std::string custom_weightfile = weightfile.generateFileName();
      std::fstream file(custom_weightfile, std::ios_base::out | std::ios_base::trunc);

      std::vector<double> value(nBins, 0);
      if (m_specific_options.m_mode == "probability") {
        for (unsigned int iBin = 0; iBin < nBins; ++iBin) {
          if (binning.m_signal_pdf[iBin] > 0)
            value[iBin] = binning.m_signal_pdf[iBin] / (binning.m_signal_pdf[iBin] + binning.m_bckgrd_pdf[iBin]);
        }
      } else {
        B2ERROR("Unkown mode in PDF Teacher named " << m_specific_options.m_mode);
        throw std::runtime_error("Unkown mode in PDF Teacher named " + m_specific_options.m_mode);
      }

      file << nBins << std::endl;
      for (unsigned int iBin = 0; iBin < nBins; ++iBin) {
        file << value[iBin] << " " << binning.m_boundaries[iBin] << std::endl;
      }
      file << binning.m_boundaries[nBins];

      file.close();

      weightfile.addOptions(m_general_options);
      weightfile.addOptions(m_specific_options);
      weightfile.addFile("PDF_Weightfile", custom_weightfile);
      weightfile.addSignalFraction(training_data.getSignalFraction());

      return weightfile;

    }

    void PDFExpert::load(Weightfile& weightfile)
    {

      std::string custom_weightfile = weightfile.generateFileName();
      weightfile.getFile("PDF_Weightfile", custom_weightfile);
      std::fstream file(custom_weightfile, std::ios_base::in);

      unsigned int nBins = 0;
      file >> nBins;
      m_value.resize(nBins, 0);

      std::vector<float> boundaries(nBins + 1, 0);

      for (unsigned int iBin = 0; iBin < nBins; ++iBin) {
        file >> m_value[iBin] >> boundaries[iBin];
      }
      file >> boundaries[nBins];

      m_binning.m_boundaries = boundaries;

      file.close();

      weightfile.getOptions(m_specific_options);
    }

    std::vector<float> PDFExpert::apply(Dataset& test_data) const
    {
      std::vector<float> probabilities(test_data.getNumberOfEvents(), 0);
      for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
        test_data.loadEvent(iEvent);
        unsigned int bin = m_binning.getBin(test_data.m_input[0]);
        probabilities[iEvent] = m_value[bin];
      }
      return probabilities;
    }

  }
}
