/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <mva/modules/MVAExpert/MVAPrototypeModule.h>
#include <mva/interface/Interface.h>

#include <boost/algorithm/string/predicate.hpp>
#include <memory>

namespace Belle2 {

  REG_MODULE(MVAPrototype)

  MVAPrototypeModule::MVAPrototypeModule() : Module()
  {
    setDescription("Prototype of a module which uses the MVA package");

    // Usually it is save to execute the MVA expert in Parallel mode,
    // but ultimately this depends on the backend you use.
    // The default method FastBDT has no problem with parallel execution.
    setPropertyFlags(c_ParallelProcessingCertified);

    // Your module probably has a parameter which defines the database identifier or filename of the mva weightfile.
    // Of course you could also hard-code it in the source code if this is not configurable
    addParam("identifier", m_identifier, "The database identifier or filename which is used to load the weights during the training.");

    // For classification it is often useful to be able to change the signalFraction.
    // If the signal fraction in the training is different from the dataset you want to apply it to,
    // you have to pass the correct signalFraction otherwise you cannot interpret the output of the classifier as a probability.
    // On the other hand, if you don't require the output to be a probability you don't have to care about this.
    addParam("signalFraction", m_signal_fraction_override,
             "signalFraction to calculate probability (if -1 the signalFraction of the training data is used)", -1.0);
  }

  void MVAPrototypeModule::initialize()
  {
    // If the identifier does not end on .root or .xml, we are dealing with a database identifier
    // so we need to create a DBObjPtr, which will fetch the weightfile from the database
    if (not(boost::ends_with(m_identifier, ".root") or boost::ends_with(m_identifier, ".xml"))) {
      m_weightfile_representation = std::make_unique<DBObjPtr<DatabaseRepresentationOfWeightfile>>(m_identifier);
    }

    // The supported methods have to be initialized once (calling it more than once is save)
    MVA::AbstractInterface::initSupportedInterfaces();

  }

  void MVAPrototypeModule::beginRun()
  {

    // If the DBObjPtr is valid we are dealing with a weightfile from the database
    if (m_weightfile_representation) {
      // We check if the weightfile changed and we have to update the expert
      if (m_weightfile_representation->hasChanged()) {
        // The actual weightfile is stored in the m_data field of the m_weightfile_representation
        std::stringstream ss((*m_weightfile_representation)->m_data);
        auto weightfile = MVA::Weightfile::loadFromStream(ss);
        init_mva(weightfile);
      }
      // In case of a file-based weightfile we load it here
      // in principal this could be done in initialize as well
    } else {
      auto weightfile = MVA::Weightfile::loadFromFile(m_identifier);
      init_mva(weightfile);
    }

  }

  void MVAPrototypeModule::init_mva(MVA::Weightfile& weightfile)
  {
    // This function initializes the MVA::Expert using the provided weightfile

    // First we get the GeneralOptions from the weightfile
    // and update the signal_fraction if required
    MVA::GeneralOptions general_options;
    weightfile.getOptions(general_options);
    if (m_signal_fraction_override > 0)
      weightfile.addSignalFraction(m_signal_fraction_override);

    // Secondly we load all supported interfaces, and fetch the correct MVA::Expert
    // and load the weightfile into this expert
    auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
    m_expert = supported_interfaces[general_options.m_method]->getExpert();
    m_expert->load(weightfile);

    // Finally, we create an MVA::SingleDataset, in which we will save our features later
    // to pass them to the expert.
    // If you want to apply the expert to more than one sample, you can also use
    // MVA::MultiDataset or any other Dataset defined by the mva package interface.
    std::vector<float> dummy(general_options.m_variables.size(), 0);
    m_dataset = std::make_unique<MVA::SingleDataset>(general_options, dummy, 0);

  }

  void MVAPrototypeModule::event()
  {
    // Just to be save we check again if the MVA::Expert is loaded
    // It can happen that for example the database doesn't find the payload
    // and the expert ends up uninitialized.
    if (not m_expert) {
      B2ERROR("MVA Expert is not loaded! I will return 0");
      return;
    }

    // You have to fill the dataset with your data.
    // The order must be the same as the order of the variables in general_options.m_variables
    for (unsigned int i = 0; i < m_dataset->getNumberOfFeatures(); ++i) {
      m_dataset->m_input[i] = 1.0;
    }

    // All what is left to do is applying the expert to the dataset
    // it will return an std::vector with the results, with one entry per sample.
    // The MVA::SingleDataset only contains one entry, so we are interested only in the first entry here.
    // The MVA::MultiDataset on the other hand would have more than one entry in the returned vector of apply.
    float probability = m_expert->apply(*m_dataset)[0];
    B2INFO("The probability is " << probability);
  }

} // Belle2 namespace

