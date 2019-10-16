from collections import namedtuple
import json

prompt_script_package = "prompt.calibrations."
prompt_script_dir = "calibration/scripts/prompt/calibrations"


class CalibrationSettings(namedtuple('CalSet_Factory', ["name", "expert_username", "description",
                                                        "input_data_formats", "input_data_names", "depends_on"])):
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
            data files.

        depends_on list(CalibrationSettings): The settings variables of the other prompt calibrations that you want
            want to depend on. This will allow the external automatic system to understand the overall ordering of
            scripts to run. If you encounter an import error when trying to run your prompt calibration script, it is
            likely that you have introduced a circular dependency.
    """

    #: Allowed data file formats. You should use these values for `CalibrationSettings.input_data_formats`.
    allowed_data_formats = frozenset({"raw", "cdst", "mdst", "udst"})

    def __new__(cls, name, expert_username, description,
                input_data_formats=None, input_data_names=None, depends_on=None):
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

        if depends_on:
            for calibration_settings in depends_on:
                if not isinstance(calibration_settings, cls):
                    raise ValueError("A list of {str(cls)} object is required when setting the 'depends_on' keyword.")
        else:
            depends_on = []

        return super().__new__(cls, name, expert_username, description, input_data_formats, input_data_names, depends_on)

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
                           "depends_on": list(depends_on_names),
                           "description": self.description
                           })

    def __str__(self):
        depends_on_names = [calibration_settings.name for calibration_settings in self.depends_on]
        output_str = str(self.__class__.__name__) + f"(name='{self.name}'):\n"
        output_str += f"  expert_username='{self.expert_username}'\n"
        output_str += f"  input_data_formats={list(self.input_data_formats)}\n"
        output_str += f"  input_data_names={list(self.input_data_names)}\n"
        output_str += f"  depends_on={list(depends_on_names)}\n"
        output_str += f"  description='{self.description}'"
        return output_str
