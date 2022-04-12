/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mva/interface/Interface.h>
#include <mva/interface/Options.h>
#include <mva/interface/Weightfile.h>

namespace Belle2::MVA {

  /**
   * Wrapper class for some utility functions.
   */
  class Utility {

  public:

    /**
     * Convenience function which downloads a given weightfile from the database
     * @param identifier identifier in the database
     * @param filename of the weightfile
     * @param experiment current experiment
     * @param run current run
     * @param event current event
     */
    static void download(const std::string& identifier, const std::string& filename, int experiment = 0, int run = 0, int event = 0);

    /**
     * Convenience function which uploads a given weightfile to the database
     * @param filename of the weightfile
     * @param identifier identifier in the database
     * @param exp1 first valid experiment
     * @param run1 first valid run
     * @param exp2 last valid experiment
     * @param run2 last valid run
     */
    static void upload(const std::string& filename, const std::string& identifier, int exp1 = 0, int run1 = 0, int exp2 = -1,
                       int run2 = -1);

    /**
     * Convenience function which uploads an array of weightfiles to the database
     * @param filenames array of names of the weightfiles
     * @param identifier identifier in the database
     * @param exp1 first valid experiment
     * @param run1 first valid run
     * @param exp2 last valid experiment
     * @param run2 last valid run
     */
    static void upload_array(const std::vector<std::string>& filenames, const std::string& identifier, int exp1 = 0, int run1 = 0,
                             int exp2 = -1, int run2 = -1);

    /**
     * Convenience function which checks if an experise is available
     * @param filename filename or identifier of the expertise
     * @param experiment current experiment
     * @param run current run
     * @param event current event
     */
    static bool available(const std::string& filename, int experiment = 0, int run = 0, int event = 0);

    /**
     * Convenience function which extracts the expertise in a given weightfile into a temporary directory
     * @param filename of the weightfile
     * @param directory temporary directory to use
     */
    static void extract(const std::string& filename, const std::string& directory);

    /**
     * Print information about the classifier stored in the given weightfile
     * @param filename filename of the weightfile
     */
    static std::string info(const std::string& filename);

    /**
     * Convenience function which saves a pre-existing weightfile in a mva package-compliant format.
     * @param general_options shared options
     * @param specific_options method specific options
     * @param custom_weightfile path to the pre-existing weightfile
     * @param output_identifier an optional string to append to the output file name. By default the function overwrites the input file.
     */
    static void save_custom_weightfile(const GeneralOptions& general_options, const SpecificOptions& specific_options,
                                       const std::string& custom_weightfile, const std::string& output_identifier = "");

    /**
     * Convenience function which performs a training with the given options
     * @param general_options shared options
     * @param specific_options method specific options
     * @param meta_options optional options
     */
    static void teacher(const GeneralOptions& general_options, const SpecificOptions& specific_options,
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
    static void expert(const std::vector<std::string>& filenames, const std::vector<std::string>& datafiles,
                       const std::string& treename,
                       const std::string& outputfile, int experiment = 0, int run = 0, int event = 0, bool copy_target = true);

    /**
     * Convenience function which performs a training on a dataset
     * @param general_options shared options
     * @param specific_options method specific options
     * @param data data to use
     */
    static std::unique_ptr<Belle2::MVA::Expert> teacher_dataset(GeneralOptions general_options, const SpecificOptions& specific_options,
                                                                Dataset& data);

    /**
     * Performs an splot training, convenience function
     * @param general_options shared options of all methods
     * @param specific_options of the used mva method
     * @param meta_options optional options
     */
    static std::unique_ptr<Belle2::MVA::Expert> teacher_splot(const GeneralOptions& general_options,
                                                              const SpecificOptions& specific_options,
                                                              const MetaOptions& meta_options);

    /**
     * Performs a sideband subtraction training, convenience function
     * @param general_options shared options of all methods
     * @param specific_options of the used mva method
     * @param meta_options optional options
     */
    static std::unique_ptr<Belle2::MVA::Expert> teacher_sideband_subtraction(const GeneralOptions& general_options,
        const SpecificOptions& specific_options,
        const MetaOptions& meta_options);

    /**
     * Performs a MC vs data pre-training and afterwards reweighted training, convenience function
     * @param general_options shared options of all methods
     * @param specific_options of the used mva method
     * @param meta_options options defining the splot training
     */
    static std::unique_ptr<Belle2::MVA::Expert> teacher_reweighting(const GeneralOptions& general_options,
        const SpecificOptions& specific_options,
        const MetaOptions& meta_options);

  };

}
