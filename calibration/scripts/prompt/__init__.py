##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
from collections import namedtuple
import json

prompt_script_package = "prompt.calibrations."
prompt_script_dir = "calibration/scripts/prompt/calibrations"

prompt_validation_script_package = "prompt.validations."
prompt_validation_script_dir = "calibration/scripts/prompt/validations"

INPUT_DATA_FILTERS = {"Magnet": {"On": "On",
                                 "Off": "Off",
                                 "Either": "Either"},
                      "Beam Energy": {"No Beam": "No Beam",
                                      "4S": "4S",
                                      "Continuum": "Continuum",
                                      "Scan": "Scan"},
                      "Run Type": {"beam": "beam",
                                   "cosmic": "cosmic",
                                   "debug": "debug", "null": "null",
                                   "physics": "physics"},
                      "Data Tag": {"hlt_skim": "hlt_skim",
                                   "bhabha_all_calib": "bhabha_all_calib",
                                   "cosmic_calib": "cosmic_calib",
                                   "gamma_gamma_calib": "gamma_gamma_calib",
                                   "hadron_calib": "hadron_calib",
                                   "mumutight_or_highm_calib": "mumutight_or_highm_calib",
                                   "offip_calib": "offip_calib",
                                   "radmumu_calib": "radmumu_calib",
                                   "random_calib": "random_calib",
                                   "single_gamma_mc": "single_gamma_mc"},
                      "Data Quality Tag": {">=30 Minute Run": ">=30 Minute Run",
                                           "Bad For Alignment": "Bad For Alignment",
                                           "Good": "Good",
                                           "Good Shifter": "Good Shifter",
                                           "Good For PXD": "Good For PXD",
                                           "Good Or Recoverable": "Good Or Recoverable",
                                           "Good Or Recoverable Shifter": "Good Or Recoverable Shifter"}
                      }


class CalibrationSettings(namedtuple('CalSet_Factory',
                                     ["name",
                                      "expert_username",
                                      "description",
                                      "input_data_formats",
                                      "input_data_names",
                                      "input_data_filters",
                                      "depends_on",
                                      "expert_config"])):
    """
    Simple class to hold and display required information for a prompt calibration script (process).

    Parameters:
        name (str): The unique calibration name, not longer than 64 characters.

        expert_username (str): The JIRA username of the expert to contact about this script.
            This username will be used to assign the default responsible person for submitting and checking prompt
            calibration jobs.

        description (str): Long form description of the calibration and what it does. Feel free to make this as long as you need.

        input_data_formats (frozenset(str)): The data formats {'raw', 'cdst', 'mdst', 'udst'} of the input files
            that should be used as input to the process. Used to figure out if this calibration should occur
            before the relevant data production e.g. before cDST files are created.

        input_data_names (frozenset(str)): The names that you will use when accessing the input data given to the
            prompt calibration process i.e. Use these in the ``get_calibrations`` function to access the correct input
            data files. e.g. input_data_names=["all_events", "offres_photon_events"]

        input_data_filters (dict): The data selection for the data input names, used for automated calibration.
            The keys should correspond to one of the ``input_data_names`` with the values being a list of the various data
            filters, e.g. Data Tag, Beam Energy, Run Type, Run Quality Tag and Magnet. All available filters can be found in the
            input_data_filters dictionary e.g. from prompt import input_data_filters with details about data tags and run quality
            tags found at: https://calibration.belle2.org/belle2/data_tags/list/.
            To exclude specific filters, pre-append with *NOT* e.g.
            {"all_events": ["mumutight_or_highm_calib", "hadron_calib", "Good", "On"],
            "offres_photon_events": ["gamma_gamma_calib", "Good", "NOT On"]}.
            Not selecting a specfic filters (e.g. Magnet) is equivalent to not having any requirements, e.g. (Either)

        depends_on (list(CalibrationSettings)): The settings variables of the other prompt calibrations that you want
            want to depend on. This will allow the external automatic system to understand the overall ordering of
            scripts to run. If you encounter an import error when trying to run your prompt calibration script, it is
            likely that you have introduced a circular dependency.

        expert_config (dict): Default expert configuration for this calibration script. This is an optional dictionary
            (which must be JSON compliant) of configuration options for your get_calibrations(...) function.
            This is supposed to be used as a catch-all place to send in options for your calibration setup. For example,
            you may want to have an optional list of IoV boundaries so that your prompt script knows that it should split the
            input data between different IoV ranges. Or you might want to send if options like the maximum events per
            input file to process. The value in your settings object will be the *default*, but you can override the value via
            the caf_config.json sent into ``b2caf-prompt-run``.
    """

    #: Allowed data file formats. You should use these values for `CalibrationSettings.input_data_formats`.
    #  Right now you should only use "raw" or "cdst" because we don't actually run calibrations on "mdst" or "udst".
    #  They are here for completeness.
    allowed_data_formats = frozenset({"raw", "cdst", "mdst", "udst"})

    def __new__(cls, name, expert_username, description,
                input_data_formats=None, input_data_names=None, input_data_filters=None, depends_on=None, expert_config=None):
        """
        The special method to create the tuple instance. Returning the instance
        calls the __init__ method
        """
        if len(name) > 64:
            raise ValueError("name cannot be longer than 64 characters!")
        if not input_data_formats:
            raise ValueError("You must specify at least one input data format")
        input_data_formats = frozenset(map(lambda x: x.lower(), input_data_formats))
        if input_data_formats.difference(cls.allowed_data_formats):
            raise ValueError("There was a data format that is not in the allowed_data_formats attribute.")
        if not input_data_names:
            raise ValueError("You must specify at least one input data name")
        input_data_names = frozenset(input_data_names)

        # The input data names in the filters MUST correspond to the input data names for the calibration.
        if input_data_filters:
            if set(input_data_filters.keys()) != input_data_names:
                raise ValueError("The 'input_data_filters' keys don't match the 'input_data_names'!")
            # Requested input data filters MUST exist in the ones we defined in the global dictionary.
            allowed_filters = {filter_name for category in INPUT_DATA_FILTERS.values() for filter_name in category}
            requested_filters = {filter_name.replace("NOT", "", 1).lstrip() for filters in input_data_filters.values()
                                 for filter_name in filters}
            if not allowed_filters.issuperset(requested_filters):
                raise ValueError("The 'input_data_filters' contains unknown filter names:"
                                 f"{requested_filters.difference(allowed_filters)}")
        else:
            input_data_filters = {}

        if expert_config:
            # Check that it's a dictionary and not some other valid JSON object
            if not isinstance(expert_config, dict):
                raise TypeError("expert_config must be a dictionary")
            # Check if it is JSONable since people might put objects in there by mistake
            try:
                json.dumps(expert_config)
            except TypeError as e:
                basf2.B2ERROR("expert_config could not be serialised to JSON. "
                              "Most likely you used a non-supported type e.g. datetime.")
                raise e
        else:
            expert_config = {}

        if depends_on:
            for calibration_settings in depends_on:
                if not isinstance(calibration_settings, cls):
                    raise TypeError(f"A list of {str(cls)} object is required when setting the 'depends_on' keyword.")
        else:
            depends_on = []

        return super().__new__(cls, name, expert_username, description,
                               input_data_formats, input_data_names, input_data_filters, depends_on, expert_config)

    def json_dumps(self):
        """
        Returns:
             str: A valid JSON format string of the attributes.
        """
        depends_on_names = [calibration_settings.name for calibration_settings in self.depends_on]
        return json.dumps({"name": self.name,
                           "expert_username": self.expert_username,
                           "input_data_formats": list(self.input_data_formats),
                           "input_data_names": list(self.input_data_names),
                           "input_data_filters": self.input_data_filters,
                           "depends_on": list(depends_on_names),
                           "description": self.description,
                           "expert_config": self.expert_config
                           })

    def __str__(self):
        depends_on_names = [calibration_settings.name for calibration_settings in self.depends_on]
        output_str = str(self.__class__.__name__) + f"(name='{self.name}'):\n"
        output_str += f"  expert_username='{self.expert_username}'\n"
        output_str += f"  input_data_formats={list(self.input_data_formats)}\n"
        output_str += f"  input_data_names={list(self.input_data_names)}\n"
        output_str += f"  input_data_filters={list(self.input_data_filters)}\n"
        output_str += f"  depends_on={list(depends_on_names)}\n"
        output_str += f"  description='{self.description}'\n"
        output_str += f"  expert_config={self.expert_config}"
        return output_str


class ValidationSettings(namedtuple('ValSet_Factory', ["name", "description", "download_files", "expert_config"])):
    """
    Simple class to hold and display required information for a validation calibration script (process).

    Parameters:
        name (str): The unique name that must match the corresponding calibration, not longer than 64 characters.

        description (str): Long form description of the validation and what it does. Feel free to make this as long as you need.

        download_files (list): The names of the files you want downloaded, e.g. mycalibration_stdout. If multiple files of
        the same name are found, all files are downloaded and appended with the folder they were in.

        expert_config (dict): Default expert configuration for this validation script. This is an optional dictionary
            (which must be JSON compliant) of configuration options for validation script.
            This is supposed to be used as a catch-all place to send in options for your calibration setup. For example,
            you may want to have an optional list of IoV boundaries so that your validation script knows that it should split the
            input data between different IoV ranges. Or you might want to send if options like the maximum events per
            input file to process. The value in your settings object will be the *default*, but you can override the value via
            the caf_config.json sent into ``b2caf-prompt-run``.
    """

    def __new__(cls, name, description, download_files=None, expert_config=None):
        """
        The special method to create the tuple instance. Returning the instance
        calls the __init__ method
        """
        if len(name) > 64:
            raise ValueError("name cannot be longer than 64 characters!")

        if expert_config:
            # Check that it's a dictionary and not some other valid JSON object
            if not isinstance(expert_config, dict):
                raise TypeError("expert_config must be a dictionary")
            # Check if it is JSONable since people might put objects in there by mistake
            try:
                json.dumps(expert_config)
            except TypeError as e:
                basf2.B2ERROR("expert_config could not be serialised to JSON. "
                              "Most likely you used a non-supported type e.g. datetime.")
                raise e
        else:
            expert_config = {}

        return super().__new__(cls, name, description, download_files, expert_config)

    def json_dumps(self):
        """
        Returns:
             str: A valid JSON format string of the attributes.
        """
        return json.dumps({"name": self.name,
                           "description": self.description,
                           "download_files": self.download_files,
                           "expert_config": self.expert_config
                           })

    def __str__(self):
        output_str = str(self.__class__.__name__) + f"(name='{self.name}'):\n"
        output_str += f"  description='{self.description}'\n"
        output_str += f"  download_files='{self.download_files}'\n"
        output_str += f"  expert_config={self.expert_config}"
        return output_str
