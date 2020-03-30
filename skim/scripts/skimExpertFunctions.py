# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Expert functions
~~~~~~~~~~~~~~~~

Some helper functions to do common tasks relating to skims.
Like testing, and for skim name encoding(decoding).
"""
import subprocess
import json

import basf2 as b2
from modularAnalysis import removeParticlesNotInLists, skimOutputUdst, summaryOfLists
from skim.registry import skim_registry
import skimTestFilesInfo
# For channels in fei skim
# from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration


_all_skims = [name for _, name in skim_registry]


def encodeSkimName(skimScriptName):
    """
    Returns the appropriate 8 digit skim code that will be used as the output uDST
    file name for any give name of a skimming script.

    :param str skimScriptName: Name of the skim.
    """
    lookup_dict = {n: c for c, n in skim_registry}
    if skimScriptName not in lookup_dict:
        b2.B2ERROR("Skim Unknown. Please add your skim to skimExpertFunctions.py.")
    return lookup_dict[skimScriptName]


def decodeSkimName(skimCode):
    """
    Returns the appropriate name of the skim given a specific skim code. This is useful to determine the skim script used
    to produce a specific uDST file, given the 8-digit code  name of the file itself.

    :param str code:
    """
    lookup_dict = {c: n for c, n in skim_registry}
    if skimCode not in lookup_dict:
        b2.B2ERROR("Code Unknown. Please add your skim to skimExpertFunctions.py")
    return lookup_dict[skimCode]


def get_test_file(sampleName):
    """
    Returns the KEKcc location of files used specifically for skim testing

    Args:
        sampleName (str): Name of the sample. MC samples are named *e.g.* "MC12_chargedBGx1", "MC9_ccbarBGx0"
    Returns:
        sampleFileName (str): The path to the test file on KEKCC.
    """
    lookup_dict = {s: f for s, f in skimTestFilesInfo.kekcc_locations}
    if sampleName not in lookup_dict:
        b2.B2ERROR("Testing file for this sample and skim campaign is not available.")
    return lookup_dict[sampleName]


def get_total_infiles(sampleName):
    """
    Returns the total number of input Mdst files for a given sample. This is useful for resource estimate.

    Args:
        sampleName (str): Name of the sample. MC samples are named *e.g.* "MC12_chargedBGx1", "MC9_ccbarBGx0"
    Returns:
        nInFiles (int): Total number of input files for sample.
    """
    lookup_dict = {s: f for s, f in skimTestFilesInfo.total_input_files}
    if sampleName not in lookup_dict:
        return None
    return lookup_dict[sampleName]


def get_events_per_file(sample):
    """
    Returns an estimate for the average number of events in an input Mdst file of the given sample type.

    Args:
        sample (str): Name of the sample. MC samples are named *e.g.* "MC12_chargedBGx1", "MC9_ccbarBGx0"
    Returns:
        nEventsPerFile (int): The average number of events in file of the given sample type.
    """
    try:
        return skimTestFilesInfo.nEventsPerFile[sample]
    except KeyError:
        return None


def add_skim(label, lists, path):
    """
    create uDST skim for given lists, saving into $label.udst.root
    Particles not necessary for the given particle lists are not saved.

    Parameters:
        label (str): the registered skim name
        lists (list(str)): the list of ParticleList names that have been created by a skim list builder function
        path (basf2.Path): modules are added to this path

    """
    skimCode = encodeSkimName(label)
    skimOutputUdst(skimCode, lists, path=path)
    summaryOfLists(lists, path=path)


def setSkimLogging(path, additional_modules=[]):
    """
    Turns the log level to ERROR for  several modules to decrease
    the total size of the skim log files

    Parameters:
        skim_path (basf2.Path): modules are added to this path
        additional_modules (list(str)): an optional list of extra noisy module
            names that should be silenced
    """
    noisy_modules = ['ParticleLoader', 'ParticleVertexFitter'] + additional_modules
    for module in path.modules():
        if module.type() in noisy_modules:
            module.set_log_level(b2.LogLevel.ERROR)
    return


def ifEventPasses(cut, conditional_path, path):
    """
    If the event passes the given ``cut`` proceed to process everything in ``conditional_path``.
    Afterwards return here and continue processing with the next module.

    Arguments:
        cut (str): selection criteria which needs to be fulfilled in order to continue with ``conditional_path``
        conditional_path (basf2.Path): path to execute if the event fulfills the criteria ``cut``
        path (basf2.Path): modules are added to this path
    """
    eselect = path.add_module("VariableToReturnValue", variable=f"passesEventCut({cut})")
    eselect.if_value('=0', conditional_path, b2.AfterConditionPath.CONTINUE)


def get_eventN(fileName):
    """
    Returns the number of events in a specific file

    Arguments:
     filename: Name of the file as clearly indicated in the argument's name.
    """

    process = subprocess.Popen(['b2file-metadata-show', '--json', fileName], stdout=subprocess.PIPE)
    out = process.communicate()[0]
    if process.returncode == 0:
        metadata = json.loads(out)
        nevents = metadata['nEvents']
        return nevents
    else:
        b2.B2ERROR("FILE INVALID OR NOT FOUND.")


def skimOutputMdst(skimDecayMode, path=None, skimParticleLists=[], outputParticleLists=[], includeArrays=[], *,
                   outputFile=None, dataDescription=None):
    """
    Create a new path for events that contain a non-empty particle list specified via skimParticleLists.
    Write the accepted events as a mdst file, saving only particles from skimParticleLists
    and from outputParticleLists. It outputs a .mdst file.
    Additional Store Arrays and Relations to be stored can be specified via includeArrays
    list argument.

    :param str skimDecayMode: Name of the skim. If no outputFile is given this is
        also the name of the output filename. This name will be added to the
        FileMetaData as an extra data description "skimDecayMode"
    :param list(str) skimParticleLists: Names of the particle lists to skim for.
        An event will be accepted if at least one of the particle lists is not empty
    :param list(str) outputParticleLists: Names of the particle lists to store in
        the output in addition to the ones in skimParticleLists
    :param list(str) includeArrays: datastore arrays/objects to write to the output
        file in addition to mdst and particle information
    :param basf2.Path path: Path to add the skim output to. Defaults to the default analysis path
    :param str outputFile: Name of the output file if different from the skim name
    :param dict dataDescription: Additional data descriptions to add to the output file. For example {"mcEventType":"mixed"}
    """
    # Note: Keep this import! `skimExpertFunctions.py` a module used commonly throughout
    # the skim package, and importing this up the top hijacks the argparser of any
    # script which imports it.
    from mdst import add_mdst_output

    # if no outputfile is specified, set it to the skim name
    if outputFile is None:
        outputFile = skimDecayMode

    # make sure the output filename has the correct extension
    if not outputFile.endswith(".mdst.root"):
        outputFile += ".mdst.root"

    skimfilter = b2.register_module('SkimFilter')
    skimfilter.set_name('SkimFilter_' + skimDecayMode)
    skimfilter.param('particleLists', skimParticleLists)
    path.add_module(skimfilter)
    filter_path = b2.create_path()
    skimfilter.if_value('=1', filter_path, b2.AfterConditionPath.CONTINUE)

    # add_independent_path() is rather expensive, only do this for skimmed events
    skim_path = b2.create_path()
    saveParticleLists = skimParticleLists + outputParticleLists
    removeParticlesNotInLists(saveParticleLists, path=skim_path)

    # set dataDescription: dictionary is mutable and thus not a good
    # default argument.
    if dataDescription is None:
        dataDescription = {}

    dataDescription.setdefault("skimDecayMode", skimDecayMode)
    add_mdst_output(outputFile, path)
    filter_path.add_independent_path(skim_path, "skim_" + skimDecayMode)


class BaseSkim:
    """Base class for skims. Initialises a skim name, and creates template functions
    required for each skim.

    To write a skim using this class:

    1. Define an object which inherits from `BaseSkim`.

    2. List all required particle lists in the attribute `RequiredParticleLists`. If we
       require the following lists to be loaded,

           >>> from stdCharged import stdK, stdPi
           >>> stdK("all", path=path)
           >>> stdK("loose", path=path)
           >>> stdPi("all", path=path)

       then `BaseSkim` will run run this code for us if `RequiredParticleLists` has the
       following value:

           {"stdCharged": {"stdK": ["all", "loose"], "stdPi": ["all"]}}

    3. If any further setup is required, then overwrite the `setup` method. This is not
       a mandatory step.

    4. Define all cuts by overwriting `build_lists`. This step is mandatory. Before the
       end of the `build_lists` method, the attribute `self.SkimLists` must be

    5. If any modules are producing too much noise, then overwrite the attribute
       `NoisyModules` as a list of those modules.

    Calling an instance of a skim class will run the particle list loaders, setup
    function, list builder function, and uDST output function. So a minimal skim
    steering file might consist of the following:

        import basf2 as b2
        import modularAnalysis as ma
        from skim.foo import MyDefinedSkim

        path = b2.Path()
        ma.inputMdstList("default", InputFiles, path=path)
        skim = MyDefinedSkim()
        skim(path)
        path.process()

    You may also want to set the __author__ attribute, so users know who to contact
    about a particular skim.
    """

    NoisyModules = []
    RequiredParticleLists = {"": {"": [""]}}

    def __init__(self, OutputFileName=None):
        """Initialise the BaseSkim class.

        Parameters:
            OutputFileName (str): Name to give output uDST files. If none given, then
                defaults to eight-number skim code.
        """
        self.name = self.__class__.__name__
        self.code = encodeSkimName(self.name)
        self.OutputFileName = OutputFileName or self.code
        self.SkimLists = []

    def setup(self, path):
        """Perform any setup steps necessary before running the skim. This may include:
            * applying event-level cuts using `ifEventPasses`,
            * adding the `MCMatcherParticles` module to the path,
            * running the FEI.

        Warning:
            Particle lists should *not* be loaded in here. This should be done by
            setting the RequiredParticleLists attribute of an individual skim. This is
            crucial for avoiding loading lists twice when combining skims for
            production.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        pass

    def build_lists(self, path):
        """Create the skim lists to be saved in the output uDST. This function is where
        the main skim cuts should be applied. At the end of this method, the attribute
        `self.SkimLists` must be set so it can be used by `output_udst()`.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        b2.B2FATAL(f"No `build_lists` method defined for skim {self}!")

    def validation(self, path):
        """Create validation histograms for the skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        # TODO: Figure out how this will work
        pass

    # Everything beyond this point can remain as-is when defining a skim
    def __call__(self, path):
        """Produce the skim particle lists and write uDST file.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        self.set_skim_logging(path)
        self.load_particle_lists(path)
        self.setup(path)
        self.build_lists(path)
        self.output_udst(path)

    def __str__(self):
        return self.name

    def __name__(self):
        return self.name

    def _validate_required_particle_lists(self):
        """Verify that the RequiredParticleLists value is in the expected format.

        The expected format is dict(str -> dict(str -> list(str))). For example, suppose
        we wished to load in charged kaons with the following call:

            >>> from stdCharged import stdK
            >>> stdK("all", path=path)
            >>> stdK("loose", path=path)

        We can achieve this using the BaseSkim class using the following value for
        RequiredParticleLists:

            RequiredParticleLists = {
                "stdCharged": {
                    "stdK": ["all", "loose"]
                }
            }
        """
        try:
            for ModuleName, FunctionsAndLabels in self.RequiredParticleLists.items():
                if not (
                    isinstance(ModuleName, str) and isinstance(FunctionsAndLabels, dict)
                ):
                    raise ValueError(
                        f"Expected str -> dict for {ModuleName} -> {FunctionsAndLabels}"
                    )

                for FunctionName, labels in FunctionsAndLabels.items():
                    if not (isinstance(FunctionName, str) and isinstance(labels, list)):
                        raise ValueError(
                            f"Expected str -> list for {FunctionName} -> {labels}"
                        )

                    if not all(isinstance(label, str) for label in labels):
                        raise ValueError(f"Expected {labels} to be a list of str.")
        except ValueError as e:
            b2.B2ERROR(
                f"Invalid format of RequiredParticleLists in {str(self)} skim. "
                "Expected dict(str -> dict(str -> list(str)))."
            )
            raise e

    def set_skim_logging(self, path):
        """Turns the log level to ERROR for selected modules to decrease the total size
        of the skim log files. Additional modules can be silenced by setting the attribute
        `NoisyModules` for an individual skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        b2.set_log_level(b2.LogLevel.INFO)

        NoisyModules = ["ParticleLoader", "ParticleVertexFitter"] + self.NoisyModules

        for module in path.modules():
            if module.type() in set(NoisyModules):
                module.set_log_level(b2.LogLevel.ERROR)

    def load_particle_lists(self, path):
        """Load the required particle lists for a skim, using the specified modules and
        functions in the attribute `RequiredParticleLists`.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for ModuleName, FunctionsAndLabels in self.RequiredParticleLists.items():
            module = __import__(ModuleName)

            for FunctionName, labels in FunctionsAndLabels.items():
                ListLoader = getattr(module, FunctionName)

                if labels:
                    for label in labels:
                        # Load lists in with calls like stdE("all", path=path)
                        ListLoader(label, path=path)
                else:
                    # If list is empty, then load lists with call like
                    # loadStdSkimPi0(path=path)
                    ListLoader(path=path)

    def output_udst(self, path):
        """Write the skim particle lists to an output uDST and print a summary of the
        skim list statistics.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """

        # Make a fuss if self.SkimLists is empty
        if len(self.SkimLists) == 0:
            b2.B2ERROR(
                f"No skim list names defined in self.SkimLists for {self} skim! "
            )

        skimOutputUdst(self.OutputFileName, self.SkimLists, path=path)
        summaryOfLists(self.SkimLists, path=path)


class CombinedSkim(BaseSkim):
    """Class for creating combined skims which can be run using similar-looking methods
    to `BaseSkim` objects.

    Skims may be combined in this class in the following way:

    >>> import basf2 as b2
    >>> from skim.foo import OneSkim, TwoSkim, RedSkim, BlueSkim
    >>> path = b2.Path()
    >>> skims = CombinedSkim(OneSkim(), TwoSkim(), RedSkim(), BlueSkim())
    >>> skims(path)  # Create all skim lists and save to uDST
    """

    def __init__(self, *skims):
        # Check that we were passed only BaseSkim objects
        if not all([isinstance(skim, BaseSkim) for skim in skims]):
            raise NotImplementedError(
                "Must pass only `BaseSkim` type objects to `CombinedSkim`."
            )

        self.Skims = skims
        self.NoisyModules = list({mod for skim in skims for mod in skim.NoisyModules})

        for skim in skims:
            skim._validate_required_particle_lists()

        self.RequiredParticleLists = self._merge_nested_dicts(
            [skim.RequiredParticleLists for skim in skims]
        )

    def __str__(self):
        # TODO: come up with a __str__ method
        pass

    def __name__(self):
        # TODO: come up with a __name__ method
        pass

    def __call__(self, path):
        self.set_skim_logging(path)
        self.load_particle_lists(path)
        self.setup(path)
        self.build_lists(path)
        self._check_duplicate_list_names()
        self.output_udst(path)

    def setup(self, path):
        """Run all of the setup functions for each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self.Skims:
            skim.setup(path)

    def build_lists(self, path):
        """Run all the build_lists functions for each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self.Skims:
            skim.build_lists(path)

    def output_udst(self, path):
        """Write uDST output files for each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self.Skims:
            skim.output_udst(path)

    def _check_duplicate_list_names(self):
        """Check for duplicate particle list names.

        Skims cannot be relied on to define their particle list names in advance, so
        this function can only be run after the build_lists() functions are run.
        """
        ParticleListLists = [skim.SkimLists for skim in self.Skims]
        ParticleLists = [l for L in ParticleListLists for l in L]
        DuplicatedParticleLists = {
            ParticleList
            for ParticleList in ParticleLists
            if ParticleLists.count(ParticleList) > 1
        }
        if DuplicatedParticleLists:
            raise ValueError(
                f"Non-unique output particle list names in combined skim! "
                ", ".join(DuplicatedParticleLists)
            )

    def _merge_nested_dicts(self, dicts):
        """Merge any number of dicts recursively. Utility function for merging
        RequiredParticleLists values from differnt skims."""
        MergedDict = dicts[0]
        for d in dicts[1:]:
            MergedDict = self._merge_two_nested_dicts(MergedDict, d)
        return MergedDict

    def _merge_two_nested_dicts(self, d1, d2):
        """Merge two dicts recursively.

        Input dicts must have the same data structure, and the innermost values must be
        lists. Lists are merged with duplictes removed.
        """
        for k in set(d1.keys()).union(d2.keys()):
            if k in d1 and k in d2:
                if isinstance(d1[k], dict) and isinstance(d2[k], dict):
                    # If we are looking at two dicts, call this function again.
                    d1[k] = self._merge_two_nested_dicts(d1[k], d2[k])
                elif isinstance(d1[k], list) and isinstance(d2[k], list):
                    # If we are looking at two lists, return a list with duplicates removed.
                    # Raise a warning if one list is empty and the other is not.
                    if len(d1[k]) == 0 ^ len(d2[k]) == 0:
                        b2.B2WARNING(
                            "Merging empty list into non-empty list. "
                            "Some particle lists may not be loaded."
                        )
                    d1[k] = sorted({*d1[k], *d2[k]})
                else:
                    raise b2.B2ERROR(
                        "Something went wrong while merging dicts. Please "
                        "check the input dicts have the same structure."
                    )
            elif k in d2:
                d1[k] = d2[k]

        return d1
