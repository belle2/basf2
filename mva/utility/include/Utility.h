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

#include <mva/interface/Interface.h>
#include <mva/interface/Options.h>
#include <mva/interface/Weightfile.h>

namespace Belle2 {
  namespace MVA {

    /**
     * Convenience function which does nothing.
     * The only purpose of this function is to be called so that ROOT loads the dictionaries belonging to the mva package
     * This is necessary if you change the working directory in your python steering file after you import basf2_mva.
     */
    void loadRootDictionary();

    /**
     * Convenience function which downloads a given weightfile from the database
     * @param identifier identifier in the database
     * @param filename of the weightfile
     * @param experiment current experiment
     * @param run current run
     * @param event current event
     */
    void download(const std::string& identifier, const std::string& filename, int experiment = 0, int run = 0, int event = 0);

    /**
     * Convenience function which uploads a given weightfile to the database
     * @param filename of the weightfile
     * @param identifier identifier in the database
     * @param exp1 first valid experiment
     * @param run1 first valid run
     * @param exp2 last valid experiment
     * @param run2 last valid run
     */
    void upload(const std::string& filename, const std::string& identifier, int exp1 = 0, int run1 = 0, int exp2 = -1, int run2 = -1);

    /**
     * Convenience function which checks if an experise is available
     * @param filename or identifier of the expertise
     * @param experiment current experiment
     * @param run current run
     * @param event current event
     */
    bool available(const std::string& filename, int experiment = 0, int run = 0, int event = 0);

    /**
     * Convenience function which extracts the expertise in a given weightfile into a temporary directory
     * @param filename of the weightfile
     * @param directory temporary directory to use
     */
    void extract(const std::string& filename, const std::string& directory);

    /**
     * Print information about the classifier stored in the given weightfile
     * @param filename of the weightfile
     */
    std::string info(const std::string& filename);

    /**
     * Convenience function which performs a training with the given options
     * @param general_options shared options
     * @param specific_options method specific options
     * @param splot_options optional options for an splot training
     */
    void teacher(const GeneralOptions& general_options, const SpecificOptions& specific_options,
                 const MetaOptions& meta_options = MetaOptions());

    /**
     * Convenience function applies experts on given data
     * @param filenames vector of filenames or database identifiers
     * @param datafiles ROOT files containing the data
     * @param treename treename of ROOT file
     * @param outputfile name of the output ROOT file
     * @param experiment number of the experiment
     * @param run number of the run
     * @param event number of the event
     * @param copy_target define if the target variable should be copied, if no target is found, an exception will be
     * raised
     */
    void expert(const std::vector<std::string>& filenames, const std::vector<std::string>& datafile, const std::string& treename,
                const std::string& outputfile, int experiment = 0, int run = 0, int event = 0, bool copy_target = true);

    /**
     * Convenience function which performs a training on a dataset
     * @param general_options shared options
     * @param specific_options method specific options
     * @param data to use
     */
    std::unique_ptr<Belle2::MVA::Expert> teacher_dataset(GeneralOptions general_options, const SpecificOptions& specific_options,
                                                         Dataset& data);

    /**
     * Performs an splot training, convenience function
     * @param general_options shared options of all methods
     * @param splot_options splot options defining the splot training
     * @param specific_options of the used mva method
     */
    std::unique_ptr<Belle2::MVA::Expert> teacher_splot(const GeneralOptions& general_options, const SpecificOptions& specific_options,
                                                       const MetaOptions& meta_options);

    /**
     * Performs a sideband substraction training, convenience function
     * @param general_options shared options of all methods
     * @param splot_options splot options defining the splot training
     * @param specific_options of the used mva method
     */
    std::unique_ptr<Belle2::MVA::Expert> teacher_sideband_substraction(const GeneralOptions& general_options,
        const SpecificOptions& specific_options,
        const MetaOptions& meta_options);

    /**
     * Performs a MC vs data pre-training and afterwards reweighted training, convenience function
     * @param general_options shared options of all methods
     * @param splot_options splot options defining the splot training
     * @param specific_options of the used mva method
     */
    std::unique_ptr<Belle2::MVA::Expert> teacher_reweighting(const GeneralOptions& general_options,
                                                             const SpecificOptions& specific_options,
                                                             const MetaOptions& meta_options);


  }
}
