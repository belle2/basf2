import subprocess


class Backend():
    """
    Base class for backend of CAF.
    Classes derived from this will implement their own submission of basf2 jobs
    to whatever backend they describe. Common methods/properties go here.
    """
    def __init__(self):
        """
        Initialise method
        """
        pass


class Local(Backend):
    """
    Backend for local calibration processes i.e. on the same machine but in a subprocess
    """
    pass


class PBS(Backend):
    """
    Backend for submitting calibration processes to a qsub batch system
    """
    pass


class LSF(Backend):
    """
    Backend for submitting calibration processes to a bsub batch system
    """
    pass


class DIRAC(Backend):
    """
    Backend for submitting calibration processes to the grid
    """
    pass
