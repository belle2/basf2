from collections import namedtuple
import json

prompt_script_package = "prompt.calibrations."
prompt_script_dir = "calibration/scripts/prompt/calibrations"


class CalibrationSettings(namedtuple('CalSet_Factory', ['name', 'description', 'input_data_formats', 'input_data_names'])):
    """
    Simple class to hold and display required information for a prompt calibration script (process).

    Parameters:
        name (str): The unique calibration name, not longer than 64 characters.

        description (str): Long form description of the calibration and what it does. Feel free to make this as long as you need.

        input_data_formats (frozenset(str)): The data formats {'raw', 'cdst', 'mdst', 'udst'} of the input files
            that should be used as input to the process. Used to figure out if this calibration should occurr
            before the relevant data production e.g. before cDST files are created.

        input_data_names (frozenset(str)): The names that you will use when accessing the input data given to the
            prompt calibration process i.e. Use these in the ``get_calibrations`` function to access the correct input
            data files.
    """

    #: Allowed data file formats. You should use these values for `CalibrationSettings.input_data_formats`.
    allowed_data_formats = frozenset({"raw", "cdst", "mdst", "udst"})

    def __new__(cls, name, description, input_data_formats=None, input_data_names=None):
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
        return super().__new__(cls, name, description, input_data_formats, input_data_names)

    def json_dumps(self):
        """
        Returns:
             str: A valid JSON format string of the attributes.
        """
        return json.dumps({"name": self.name,
                           "input_data_formats": list(self.input_data_formats),
                           "input_data_names": list(self.input_data_names),
                           "description": self.description
                           })

    def __str__(self):
        output_str = str(self.__class__.__name__) + f"(name='{self.name}'):\n"
        output_str += f"  input_data_formats={list(self.input_data_formats)}\n"
        output_str += f"  input_data_names={list(self.input_data_names)}\n"
        output_str += f"  description='{self.description}'"
        return output_str
