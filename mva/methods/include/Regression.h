/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <mva/interface/Dataset.h>
#include <mva/interface/Expert.h>
#include <mva/interface/Options.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Weightfile.h>

#include <framework/logging/Logger.h>

#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/program_options.hpp>
#include <vector>

namespace Belle2 {
  namespace MVA {

    /**
     * Generic options of the Regression MVA methods hosting the number of bins (and the base classifier options).
     * You need to override this to add a getMethod() function. See the RegressionFastBDTOptions as an example.
     */
    template<class BaseClassifierOptions>
    class RegressionOptions : public SpecificOptions {
    public:
      /// Load the options from a boost property tree. Only loads the maximal bin number.
      void load(const boost::property_tree::ptree& pt) override;

      /// Save the options from a boost property tree. Only saves the maximal bin number.
      void save(boost::property_tree::ptree& pt) const override;

      /// Get the descriptions for these options
      po::options_description getDescription() override;

      /// Return options of the base classifier (const version)
      const BaseClassifierOptions& getBaseClassifierOptions() const;

      /// Return options of the base classifier (non-const version)
      BaseClassifierOptions& getBaseClassifierOptions();

      /// Return the number of bins to use
      unsigned int getMaximalBinNumber() const;

      /// Set the number of bins to use
      void setMaximalBinNumber(unsigned int maximalBinNumber);

    private:
      /// Options of the base classifier
      BaseClassifierOptions m_baseClassifierOptions;
      /// How many bins to use
      unsigned int m_numberOfBins = 4;
    };

    /**
     * Dataset needed during the training of a regression method.
     *
     * It basically wraps another dataset it receives as pointer and every call to
     * this dataset is forewarded to the other dataset.
     *
     * The only difference is, that the isSignal variable is set dependent on the cut value
     * given in the constructor to
     *
     *      isSignal = target >= cutValue
     *
     * to generate a binary classification out of a regression task.
     */
    class RegressionDataSet : public Dataset {
    public:
      /// Create a new regression data set out of the general options, a pointer to the real dataset and the cut value
      RegressionDataSet(const GeneralOptions& general_options, Dataset* dataSet, double cutValue);

      /// Return the number of features from the real dataset
      unsigned int getNumberOfFeatures() const override;

      /// Return the number of events from the real dataset
      unsigned int getNumberOfEvents() const override;

      /// Return the number of spectators from the real dataset
      unsigned int getNumberOfSpectators() const override;

      /// Load an event. Sets all internal variables and sets the isSignal variable dependent on the cut value
      void loadEvent(unsigned int iEvent) override;

      /// Return a specific feature from the real dataset
      std::vector<float> getFeature(unsigned int iFeature) override;

      /// Return a specific spectator from the real dataset
      std::vector<float> getSpectator(unsigned int iSpectator) override;

      /// Return the weights from the real dataset
      std::vector<float> getWeights() override;

      /// Return the targets from the real dataset
      std::vector<float> getTargets() override;

    private:
      /// The cut value
      double m_cutValue;

      /// The real data set (our child)
      Dataset* m_childDataSet;
    };

    /**
     * Core class for the training of regression methods based on binary classifiers.
     *
     * The idea is as follows:
     *  * If you want to solve a regression task, you need to make sure your target is between
     *    0 and 1 and is more or less equally distributed (this is your task)
     *  * The teacher will then train N binary classifiers, where N is given by the maximal bin number
     *    in the options.
     *  * Each binary classifier is trained on the same data, but as a (binary) target is using
     *
     *      s >= (1/number of bins) * (bin number)
     *
     *    for every bin in [1, number of bins). This means, the first method is able to decide
     *    if a target value is very low or not, the second if it is medium low or not ...
     *    and the last if it is very high or not.
     *    The expert answer to a new data sample is the averaged output of all
     *    classifiers, which is very high, if all classifiers think it has a high value (then the target value
     *    is probably high) and very small, if non of them thinks the target value is high.
     *
     * The class is very generic and can be used with every binary classifier.
     *
     * @tparam BaseClassifierTeacher The teacher of the base binary classifier
     * @tparam RegressionClassifierOptions The options of the classifier (probably inheriting from RegressionOptions)
     */
    template<class BaseClassifierTeacher, class RegressionClassifierOptions>
    class RegressionTeacher : public Teacher {
    public:
      /// Create a new teacher out of the general and the regression method specific options
      RegressionTeacher(const GeneralOptions& general_options,
                        const RegressionClassifierOptions& specific_options);

      /**
       * Call the train function.
       * For each bin number in [1, number of bins) train a new classifier on the target
       *
       *    s >= (1/number of bins) * (bin number)
       *
       * where s is the target variable of the dataset.
       * By this, we end up with (number of bins) classifier, each trained on the same
       * data but with different targets.
       */
      Weightfile train(Dataset& training_data) const override;

    private:
      /// The method specific options
      RegressionClassifierOptions m_specific_options;
      /// The teacher to use for training
      BaseClassifierTeacher m_baseClassifierTeacher;
    };

    /**
     * Generic expert for the regression applications.
     * For each trained base classifier (for each bin, see the teacher for how this is done),
     * one weight file is written as part into the full weight file.
     * This expert reads all of them, instantiates a base expert for each bin
     * and applies all of them to the dataset.
     *
     * The result of the calculation is given by
     *
     *      probability = sum(expert probability for each expert) / # of experts
     *
     * @tparam BaseClassifierExpert Another expert that is related to the teacher you used for training
     * @tparam RegressionClassifierOptions The options of the classifier (probably inheriting from RegressionOptions)
     */
    template<class BaseClassifierExpert, class RegressionClassifierOptions>
    class RegressionExpert : public Expert {
    public:
      /// Load the expert from the weightfile by loading each stored single classifier one ofter the other
      void load(Weightfile& weightfile) override;

      /// Apply the loaded experts by averaging over the single expert decisions
      std::vector<float> apply(Dataset& test_data) const override;

    private:
      /// The list of single experts
      std::vector<BaseClassifierExpert> m_baseClassifierExperts;
    };
  }
}


template<class BaseClassifierOptions>
void Belle2::MVA::RegressionOptions<BaseClassifierOptions>::load(const boost::property_tree::ptree& pt)
{
  m_numberOfBins = pt.get<unsigned int>(getMethod() + "_numberOfBins");
}

template<class BaseClassifierOptions>
unsigned int Belle2::MVA::RegressionOptions<BaseClassifierOptions>::getMaximalBinNumber() const
{
  return m_numberOfBins;
}

template<class BaseClassifierOptions>
void Belle2::MVA::RegressionOptions<BaseClassifierOptions>::setMaximalBinNumber(unsigned int maximalBinNumber)
{
  m_numberOfBins = maximalBinNumber;
}

template<class BaseClassifierOptions>
const BaseClassifierOptions& Belle2::MVA::RegressionOptions<BaseClassifierOptions>::getBaseClassifierOptions() const
{
  return m_baseClassifierOptions;
}

template<class BaseClassifierOptions>
BaseClassifierOptions& Belle2::MVA::RegressionOptions<BaseClassifierOptions>::getBaseClassifierOptions()
{
  return m_baseClassifierOptions;
}

template<class BaseClassifierOptions>
void Belle2::MVA::RegressionOptions<BaseClassifierOptions>::save(boost::property_tree::ptree& pt) const
{
  pt.put(getMethod() + "_numberOfBins", m_numberOfBins);
}

template<class BaseClassifierOptions>
po::options_description Belle2::MVA::RegressionOptions<BaseClassifierOptions>::getDescription()
{
  auto description = getBaseClassifierOptions().getDescription();

  description.add_options()
  ("numberOfBins", po::value<unsigned int>(&m_numberOfBins),
   "Number of bins to split the target variable into");

  return description;
}

template<class BaseClassifierTeacher, class RegressionClassifierOptions>
Belle2::MVA::Weightfile Belle2::MVA::RegressionTeacher<BaseClassifierTeacher, RegressionClassifierOptions>::train(
  Belle2::MVA::Dataset& training_data) const
{
  Weightfile weightfile;
  weightfile.addOptions(m_general_options);
  weightfile.addOptions(m_specific_options);

  for (unsigned int binNumber = 1; binNumber < m_specific_options.getMaximalBinNumber(); binNumber++) {
    RegressionDataSet specificDataSet(m_general_options, &training_data,
                                      1.0 / m_specific_options.getMaximalBinNumber() * binNumber);
    Weightfile specificWeightFile = m_baseClassifierTeacher.train(specificDataSet);

    std::string file = weightfile.generateFileName();
    Weightfile::saveToXMLFile(specificWeightFile, file);
    weightfile.addFile("BaseClassifier_WeightFile_" + std::to_string(binNumber), file);
  }
  return weightfile;
}

template<class BaseClassifierTeacher, class RegressionClassifierOptions>
Belle2::MVA::RegressionTeacher<BaseClassifierTeacher, RegressionClassifierOptions>::RegressionTeacher(
  const Belle2::MVA::GeneralOptions& general_options,
  const RegressionClassifierOptions& specific_options) :
  Teacher(general_options), m_specific_options(specific_options),
  m_baseClassifierTeacher(general_options, specific_options.getBaseClassifierOptions())
{
}

template<class BaseClassifierExpert, class RegressionClassifierOptions>
void Belle2::MVA::RegressionExpert<BaseClassifierExpert, RegressionClassifierOptions>::load(Weightfile& weightfile)
{
  RegressionClassifierOptions specific_options;
  weightfile.getOptions(specific_options);

  m_baseClassifierExperts.resize(specific_options.getMaximalBinNumber());
  for (unsigned int binNumber = 1; binNumber < specific_options.getMaximalBinNumber(); binNumber++) {
    std::string file = weightfile.generateFileName();
    weightfile.getFile("BaseClassifier_WeightFile_" + std::to_string(binNumber), file);

    auto specificWeightFile = Weightfile::loadFromXMLFile(file);
    m_baseClassifierExperts[binNumber].load(specificWeightFile);
  }
}

template<class BaseClassifierExpert, class RegressionClassifierOptions>
std::vector<float> Belle2::MVA::RegressionExpert<BaseClassifierExpert, RegressionClassifierOptions>::apply(
  Dataset& test_data) const
{
  std::vector<float> sum;
  for (const auto& expert : m_baseClassifierExperts) {
    if (sum.empty()) {
      // First time we do not need to add something, but just replace
      sum = expert.apply(test_data);
      continue;
    }

    const auto& expertResult = expert.apply(test_data);
    if (sum.size() != expertResult.size()) {
      B2FATAL("There is one expert in the list that returned not the same number of results than the others!");
    }

    for (unsigned int index = 0; index < sum.size(); index++) {
      sum[index] += expertResult[index];
    }
  }

  for (unsigned int index = 0; index < sum.size(); index++) {
    sum[index] /= m_baseClassifierExperts.size();
  }

  return sum;
}
