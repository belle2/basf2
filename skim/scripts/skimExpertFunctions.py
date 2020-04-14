# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from abc import ABC, abstractmethod
import subprocess
from importlib import import_module
import json
import re

import basf2 as b2
from modularAnalysis import removeParticlesNotInLists, skimOutputUdst, summaryOfLists
from skim.registry import Registry
import skimTestFilesInfo
# For channels in fei skim
# from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration


_all_skims = Registry.names


def encodeSkimName(SkimName):
    """
    Returns the appropriate 8 digit skim code that will be used as the output uDST
    file name for any give name of a skimming script.

    :param str SkimName: Name of the skim.
    """
    # TODO: delete `encodeSkimName` and use `Registry.encode_skim_name` in its place
    return Registry.encode_skim_name(SkimName)


def decodeSkimName(SkimCode):
    """
    Returns the appropriate name of the skim given a specific skim code. This is useful to determine the skim script used
    to produce a specific uDST file, given the 8-digit code  name of the file itself.

    :param str code:
    """
    # TODO: delete `decodeSkimName` and use `Registry.decode_skim_code` in its place
    return Registry.decode_skim_code(SkimCode)


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
    skimCode = Registry.encode_skim_name(label)
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


def fancy_skim_header(SkimClass):
    """Decorator to generate a fancy header to skim documentation and prepend it to the
    docstring. Add this just above the definition of a skim.

    Also ensures the documentation of the template functions like `BaseSkim.build_lists`
    is not repeated in every skim documentation.

    .. code-block:: python

        @fancy_skim_header
        class MySkimName(BaseSkim):
            # docstring here describing your skim, and explaining cuts.
    """
    SkimName = SkimClass.__name__
    SkimCode = Registry.encode_skim_name(SkimName)
    authors = SkimClass.__authors__ or ["(no authors listed)"]
    description = SkimClass.__description__ or "(no description)"
    contact = SkimClass.__contact__ or "(no contact listed)"
    category = SkimClass.__category__ or "(no category listed)"

    if isinstance(authors, str):
        # If we were given a string, split it up at: commas, "and", "&", and newlines
        authors = re.split(r",\s+and\s+|\s+and\s+|,\s+&\s+|\s+&\s+|,\s+|\s*\n\s*", authors)
        # Strip any remaining whitespace either side of an author's name
        authors = [re.sub(r"^\s+|\s+$", "", author) for author in authors]

    if isinstance(category, list):
        category = ", ".join(category)

    # If the contact is of the form "NAME <EMAIL>" or "NAME (EMAIL)", then make it a link
    match = re.match("([^<>()`]+) [<(]([^<>()`]+@[^<>()`]+)[>)]", contact)
    if match:
        name, email = match[1], match[2]
        contact = f"`{name} <mailto:{email}>`_"

    header = f"""
    Note:
        * **Skim description**: {description}
        * **Skim name**: {SkimName}
        * **Skim LFN code**: {SkimCode}
        * **Category**: {category}
        * **Author{"s"*(len(authors) > 1)}**: {", ".join(authors)}
        * **Contact**: {contact}
    """

    if SkimClass.__doc__:
        SkimClass.__doc__ = header + "\n\n" + SkimClass.__doc__.lstrip("\n")
    else:
        # Handle case where docstring is empty, or was not redefined
        SkimClass.__doc__ = header

    # If documentation of template functions not redefined, make sure BaseSkim docstring is not repeated
    SkimClass.build_lists.__doc__ = SkimClass.build_lists.__doc__ or ""
    SkimClass.validation_histograms.__doc__ = SkimClass.validation_histograms.__doc__ or ""
    SkimClass.additional_setup.__doc__ = SkimClass.additional_setup.__doc__ or ""

    return SkimClass


class BaseSkim(ABC):
    """Base class for skims. Initialises a skim name, and creates template functions
    required for each skim.

    See `writing-skims` for information on how to use this to define a new skim.
    """

    NoisyModules = []
    """A list of modules which to be silenced for this skim. This may be necessary to
    set in order to keep log file sizes small."""

    TestFile = get_test_file("MC12_mixedBGx1")
    """Location of an MDST file to test the skim on. Defaults to an MC12 mixed BGx1
    sample. If you want to use a different test file for your skim, set it using
    `get_test_file`.
    """

    # Abstract method to ensure that it is overriden whenever `BaseSkim` is inherited
    @property
    @abstractmethod
    def RequiredStandardLists(self):
        """Data structure specifying the standard particle lists to be loaded in before
        constructing the skim list. Must have the form ``dict(str -> dict(str ->
        list(str)))``.

        If we require the following lists to be loaded,

               >>> from stdCharged import stdK, stdPi
               >>> from skim.standardlists.lightmesons import loadStdLightMesons
               >>> stdK("all", path=path)
               >>> stdK("loose", path=path)
               >>> stdPi("all", path=path)
               >>> loadStdLightMesons(path=path)

        then `BaseSkim` will run run this code for us if we set `RequiredStandardLists` to
        the following value:

        .. code-block:: python

            {
                "stdCharged": {
                    "stdK": ["all", "loose"],
                    "stdPi": ["all"],
                },
                "skim.standardlists.lightmesons": {
                      "loadStdLightMesons": [],
                },
            }
        """

    @property
    @abstractmethod
    def __description__(self):
        pass

    @property
    @abstractmethod
    def __category__(self):
        pass

    @property
    @abstractmethod
    def __authors__(self):
        pass

    @property
    @abstractmethod
    def __contact__(self):
        pass

    def __init__(self, *, OutputFileName=None):
        """Initialise the BaseSkim class.

        Parameters:
            OutputFileName (str): Name to give output uDST files. If none given, then
                defaults to eight-number skim code.
        """
        self.name = self.__class__.__name__
        self.code = Registry.encode_skim_name(self.name)
        self.OutputFileName = OutputFileName or self.code
        self.SkimLists = []

    def additional_setup(self, path):
        """
        Perform any setup steps necessary before running the skim. This may include:

        * applying event-level cuts using `ifEventPasses`,
        * adding the `MCMatcherParticles` module to the path,
        * running the FEI.

        Warning:
            Standard particle lists should *not* be loaded in here. This should be done
            by setting the `RequiredStandardLists` attribute of an individual skim. This
            is crucial for avoiding loading lists twice when combining skims for
            production.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """

    # Abstract method to ensure that it is overriden whenever `BaseSkim` is inherited
    @abstractmethod
    def build_lists(self, path):
        """Create the skim lists to be saved in the output uDST. This function is where
        the main skim cuts should be applied. At the end of this method, the attribute
        ``SkimLists`` must be set so it can be used by `output_udst`.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """

    def validation_histograms(self, path):
        """Create validation histograms for the skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """

    # Everything beyond this point can remain as-is when defining a skim
    def __call__(self, path):
        """Produce the skim particle lists and write uDST file.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        self.set_skim_logging(path)
        self.load_particle_lists(path)
        self.additional_setup(path)
        self.build_lists(path)
        self.output_udst(path)

    def _method_unchanged(self, method):
        """Check if the method of the class is the same as in its parent class, or if it has
        been overriden.

        Useful for determining if *e.g.* `validation_histograms` has been defined for a
        particular skim.
        """
        cls = self.__class__
        parent_cls = cls.__mro__[1]

        if hasattr(cls, method) and hasattr(parent_cls, method):
            return getattr(cls, method) == getattr(parent_cls, method)
        else:
            return False

    def __str__(self):
        return self.name

    def __name__(self):
        return self.name

    def _validate_required_particle_lists(self):
        """Verify that the `RequiredStandardLists` value is in the expected format.

        The expected format is ``dict(str -> dict(str -> list(str)))``.
        """
        if self.RequiredStandardLists is None:
            return

        try:
            for ModuleName, FunctionsAndLabels in self.RequiredStandardLists.items():
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
                f"Invalid format of RequiredStandardLists in {str(self)} skim. "
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
        functions in the attribute `RequiredStandardLists`.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        if self.RequiredStandardLists is None:
            return

        # Reorder RequiredStandardLists so skim.standardlists modules are loaded *last*
        StandardLists = {
            k: v for (k, v) in self.RequiredStandardLists.items()
            if not k.startswith("skim.standardlists.")
        }
        StandardSkimLists = {
            k: v for (k, v) in self.RequiredStandardLists.items()
            if k.startswith("skim.standardlists.")
        }
        self.RequiredStandardLists = {**StandardLists, **StandardSkimLists}

        for ModuleName, FunctionsAndLabels in self.RequiredStandardLists.items():
            module = import_module(ModuleName)

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
            b2.B2FATAL(
                f"No skim list names defined in self.SkimLists for {self} skim!"
            )

        skimOutputUdst(self.OutputFileName, self.SkimLists, path=path)
        summaryOfLists(self.SkimLists, path=path)


class CombinedSkim(BaseSkim):
    """Class for creating combined skims which can be run using similar-looking methods
    to `BaseSkim` objects.

    A steering file which combines skims can be as simple as the following:

    .. code-block:: python

        import basf2 as b2
        import modularAnalysis as ma
        from skim.foo import OneSkim, TwoSkim, RedSkim, BlueSkim

        path = b2.Path()
        ma.inputMdstList("default", [], path=path)
        skims = CombinedSkim(OneSkim(), TwoSkim(), RedSkim(), BlueSkim())
        skims(path)  # load standard lists, create skim lists, and save to uDST
        path.process()

    When skims are combined using this class, the `NoisyModules` lists of each skim are
    combined and all silenced, and the `RequiredStandardLists` objects are
    merged, removing duplicates. This way, `load_particle_lists` will load all the
    required lists of each skim, without accidentally loading a list twice.

    The heavy-lifting functions `additional_setup`, `build_lists` and `output_udst` are
    modified to loop over the corresponding functions of each invididual skim. Calling
    an instance of the `CombinedSkim` class will load all the required particle lists,
    then run all the setup steps, then the list building functions, and then all the
    output steps.
    """

    __authors__ = ["Phil Grace"]
    __WorkingGroup__ = None
    __description__ = None

    RequiredStandardLists = None
    """`BaseSkim.RequiredStandardLists` attribute initialised to `None` to get around
    abstract property restriction. Overriden as merged
    `BaseSkim.RequiredStandardLists` object during initialisation of `CombinedSkim`
    instance."""

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

        self.RequiredStandardLists = self._merge_nested_dicts(
            skim.RequiredStandardLists for skim in skims
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
        self.additional_setup(path)
        self.build_lists(path)
        self._check_duplicate_list_names()
        self.output_udst(path)

    def additional_setup(self, path):
        """Run the `BaseSkim.additional_setup` function of each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self.Skims:
            skim.additional_setup(path)

    def build_lists(self, path):
        """Run the `BaseSkim.build_lists` function of each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self.Skims:
            skim.build_lists(path)

    def output_udst(self, path):
        """Run the `BaseSkim.output_udst` function of each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self.Skims:
            skim.output_udst(path)

    def _check_duplicate_list_names(self):
        """Check for duplicate particle list names.

        .. Note::

            Skims cannot be relied on to define their particle list names in advance, so
            this function can only be run after `build_lists` is run.
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

    def _merge_nested_dicts(self, *dicts):
        """Merge any number of dicts recursively.

        Utility function for merging `RequiredStandardLists` values from differnt
        skims.

        Parameters:
            dicts (dict): Any number of dictionaries to be merged.

        Returns:
            MergedDict (dict): Merged dictionary without duplicates.
        """
        # Convert dicts from a generator expression into a list with None values removed
        dicts = list(filter(None, list(*dicts)))
        if not dicts:
            return None

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
                        # TODO: remove this warning and replace with proper handling
                        # b2.B2WARNING(
                        #     "Merging empty list into non-empty list. "
                        #     "Some particle lists may not be loaded."
                        # )
                        pass
                    d1[k] = sorted({*d1[k], *d2[k]})
                else:
                    raise b2.B2ERROR(
                        "Something went wrong while merging dicts. Please "
                        "check the input dicts have the same structure."
                    )
            elif k in d2:
                d1[k] = d2[k]

        return d1
