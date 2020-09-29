#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from abc import ABC, abstractmethod
import subprocess
import json
import re

import yaml

import basf2 as b2
from modularAnalysis import applyCuts, removeParticlesNotInLists, skimOutputUdst, summaryOfLists
from skim.registry import Registry


def _get_test_sample_info(sampleName):
    """Read in the YAML file of test samples (``skim/scripts/TestFiles.yaml``) and
    return the info for a sample as a dict.
    """

    with open(b2.find_file("skim/scripts/TestFiles.yaml")) as f:
        skimTestFilesInfo = yaml.safe_load(f)

    try:
        return skimTestFilesInfo[sampleName]
    except KeyError:
        msg = f"Sample {sampleName} not listed in skim/scripts/TestFiles.yaml."
        b2.B2ERROR(msg)
        raise KeyError(msg)


def get_test_file(sampleName):
    """
    Returns the KEKCC location of files used specifically for skim testing

    Args:
        sampleName (str): Name of the sample. MC samples are named *e.g.* "MC12_chargedBGx1", "MC9_ccbarBGx0"
    Returns:
        The path to the test file on KEKCC.
    """
    sampleInfo = _get_test_sample_info(sampleName)

    if "location" in sampleInfo and sampleInfo["location"] is not None:
        return sampleInfo["location"]
    else:
        msg = f"No test file location listed for {sampleName} sample."
        b2.B2ERROR(msg)
        raise KeyError(msg)


def get_total_infiles(sampleName):
    """
    Returns the total number of input MDST files for a given sample. This is useful for resource estimate.

    Args:
        sampleName (str): Name of the sample. MC samples are named *e.g.* "MC12_chargedBGx1", "MC9_ccbarBGx0"
    Returns:
        Total number of input files for sample.
    """
    sampleInfo = _get_test_sample_info(sampleName)

    if "total_input_files" in sampleInfo and sampleInfo["total_input_files"] is not None:
        return sampleInfo["total_input_files"]
    else:
        msg = f"'total_input_files' not listed for {sampleName} sample."
        raise KeyError(msg)


def get_events_per_file(sampleName):
    """
    Returns an estimate for the average number of events in an input MDST file of the given sample type.

    Args:
        sampleName (str): Name of the sample. MC samples are named *e.g.* "MC12_chargedBGx1", "MC9_ccbarBGx0"
    Returns:
        The average number of events in file of the given sample type.
    """
    sampleInfo = _get_test_sample_info(sampleName)

    if "average_events_per_file" in sampleInfo and sampleInfo["average_events_per_file"] is not None:
        return sampleInfo["average_events_per_file"]
    else:
        msg = f"'average_events_per_file' not listed for {sampleName} sample."
        raise KeyError(msg)


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


def setSkimLogging(path, additional_modules=None):
    """
    Turns the log level to ERROR for  several modules to decrease
    the total size of the skim log files

    Parameters:
        skim_path (basf2.Path): modules are added to this path
        additional_modules (list(str)): an optional list of extra noisy module
            names that should be silenced
    """
    if additional_modules is None:
        additional_modules = []
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
    eselect.if_value('=1', conditional_path, b2.AfterConditionPath.CONTINUE)


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


def skimOutputMdst(skimDecayMode, path=None, skimParticleLists=None, outputParticleLists=None, includeArrays=None, *,
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

    if skimParticleLists is None:
        skimParticleLists = []
    if outputParticleLists is None:
        outputParticleLists = []
    if includeArrays is None:
        includeArrays = []

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
    add_mdst_output(skim_path, filename=outputFile)
    filter_path.add_independent_path(skim_path, "skim_" + skimDecayMode)


def _sphinxify_decay(decay_string):
    """Format the given decay string by using LaTeX commands instead of plain-text.
    Output is formatted for use with Sphinx (ReStructured Text).

    This is a utility function for autogenerating skim documentation.

    Parameters:
        decay_string (str): A decay descriptor.

    Returns:
        sphinxed_string (str): LaTeX version of the decay descriptor.
    """

    decay_string = re.sub("^(B.):generic", "\\1_{\\\\text{had}}", decay_string)
    decay_string = decay_string.replace(":generic", "")
    decay_string = decay_string.replace(":semileptonic", "_{\\text{SL}}")
    decay_string = decay_string.replace(":FSP", "_{FSP}")
    decay_string = decay_string.replace(":V0", "_{V0}")
    decay_string = re.sub("_[0-9]+", "", decay_string)
    # Note: these are applied from top to bottom, so if you have
    # both B0 and anti-B0, put anti-B0 first.
    substitutes = [
        ("==>", "\\to"),
        ("->", "\\to"),
        ("gamma", "\\gamma"),
        ("p+", "p"),
        ("anti-p-", "\\bar{p}"),
        ("pi+", "\\pi^+"),
        ("pi-", "\\pi^-"),
        ("pi0", "\\pi^0"),
        ("K_S0", "K^0_S"),
        ("K_L0", "K^0_L"),
        ("mu+", "\\mu^+"),
        ("mu-", "\\mu^-"),
        ("tau+", "\\tau^+"),
        ("tau-", "\\tau^-"),
        ("nu", "\\nu"),
        ("K+", "K^+"),
        ("K-", "K^-"),
        ("e+", "e^+"),
        ("e-", "e^-"),
        ("J/psi", "J/\\psi"),
        ("anti-Lambda_c-", "\\Lambda^{-}_{c}"),
        ("anti-Sigma+", "\\overline{\\Sigma}^{+}"),
        ("anti-Lambda0", "\\overline{\\Lambda}^{0}"),
        ("anti-D0*", "\\overline{D}^{0*}"),
        ("anti-D*0", "\\overline{D}^{0*}"),
        ("anti-D0", "\\overline{D}^0"),
        ("anti-B0", "\\overline{B}^0"),
        ("Sigma+", "\\Sigma^{+}"),
        ("Lambda_c+", "\\Lambda^{+}_{c}"),
        ("Lambda0", "\\Lambda^{0}"),
        ("D+", "D^+"),
        ("D-", "D^-"),
        ("D0", "D^0"),
        ("D*+", "D^{+*}"),
        ("D*-", "D^{-*}"),
        ("D*0", "D^{0*}"),
        ("D_s+", "D^+_s"),
        ("D_s-", "D^-_s"),
        ("D_s*+", "D^{+*}_s"),
        ("D_s*-", "D^{-*}_s"),
        ("B+", "B^+"),
        ("B-", "B^-"),
        ("B0", "B^0"),
        ("B_s0", "B^0_s"),
        ("K*0", "K^{0*}")]
    tex_string = decay_string
    for (key, value) in substitutes:
        tex_string = tex_string.replace(key, value)
    return f":math:`{tex_string}`"


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

    if SkimClass.ApplyHLTHadronCut:
        HLTLine = "*This skim includes a selection on the HLT flag* ``hlt_hadron``."
        header = f"{header.rstrip()}\n\n        {HLTLine}\n"

    if SkimClass.__doc__:
        SkimClass.__doc__ = header + "\n\n" + SkimClass.__doc__.lstrip("\n")
    else:
        # Handle case where docstring is empty, or was not redefined
        SkimClass.__doc__ = header

    # If documentation of template functions not redefined, make sure BaseSkim docstring is not repeated
    SkimClass.load_standard_lists.__doc__ = SkimClass.load_standard_lists.__doc__ or ""
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

    TestFiles = [get_test_file("MC13_mixedBGx1")]
    """Location of an MDST file to test the skim on. Defaults to an MC13 mixed BGx1
    sample. If you want to use a different test file for your skim, set it using
    `get_test_file`.
    """

    MergeDataStructures = {}
    """Dict of ``str -> function`` pairs to determine if any special data structures
    should be merged when combining skims. Currently, this is only used to merge FEI
    config parameters when running multiple FEI skims at once, so that it can be run
    just once with all the necessary arguments."""

    ApplyHLTHadronCut = False
    """If this property is set to True, then the HLT selection for ``hlt_hadron`` will
    be applied to the skim lists when the skim is added to the path.
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

    def load_standard_lists(self, path):
        """
        Load any standard lists. This code will be run before any
        `BaseSkim.additional_setup` and `BaseSkim.build_lists`.

        Note:
            This is separated into its own function so that when skims are combined, any
            standard lists used by two skims can be loaded just once.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """

    def additional_setup(self, path):
        """
        Perform any setup steps necessary before running the skim. This may include:

        * applying event-level cuts using `ifEventPasses`,
        * adding the `MCMatcherParticles` module to the path,
        * running the FEI.

        Warning:
            Standard particle lists should *not* be loaded in here. This should be done
            by overriding the method `BaseSkim.load_standard_lists`. This is crucial for
            avoiding loading lists twice when combining skims for production.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """

    # Abstract method to ensure that it is overridden whenever `BaseSkim` is inherited
    @abstractmethod
    def build_lists(self, path):
        """Create the skim lists to be saved in the output uDST. This function is where
        the main skim cuts should be applied. At the end of this method, the attribute
        ``SkimLists`` must be set so it can be used by `BaseSkim.output_udst`.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """

    def validation_histograms(self, path):
        """Create validation histograms for the skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """

    # Everything beyond this point can remain as-is when defining a skim
    def __call__(self, path, *, udstOutput=True, validation=False):
        """Produce the skim particle lists and write uDST file.

        Parameters:
            path (basf2.Path): Skim path to be processed.
            udstOutput (bool): If True, add uDST output to the path.
            validation (bool): If True, build lists and write validation histograms
                instead of writing uDSTs.
        """
        self.set_skim_logging(path)
        self.load_standard_lists(path)
        self.additional_setup(path)
        # At this point, BaseSkim.skim_event_cuts may have been run, so pass
        # self._ConditionalPath for the path if it is not None (otherwise just pass the
        # regular path)
        self.build_lists(self._ConditionalPath or path)
        self.apply_hlt_hadron_cut(self._ConditionalPath or path)

        if udstOutput:
            self.output_udst(self._ConditionalPath or path)

        if validation:
            if self._method_unchanged("validation_histograms"):
                b2.B2FATAL(f"No validation histograms defined for {self} skim.")
            self.validation_histograms(self._ConditionalPath or path)

    _ConditionalPath = None
    """Conditional path to be set by `BaseSkim.skim_event_cuts` if event-level cuts are applied."""

    def skim_event_cuts(self, cut, *, path):
        """Apply event-level cuts in a skim-safe way.

        Parameters:
            cut (str): Event-level cut to be applied.
            path (basf2.Path): Skim path to be processed.

        Returns:
            Path on which the rest of this skim should be processed.
            On this path, only events which passed the event-level cut will
            be processed further.

        .. Tip::
            If running this function in `BaseSkim.additional_setup` or
            `BaseSkim.build_lists`, redefine the ``path`` to the path returned by
            `BaseSkim.skim_event_cuts`, *e.g.*

            .. code-block:: python

                def build_lists(self, path):
                    path = self.skim_event_cuts("nTracks>4", path=path)
                    # rest of skim list building...

        .. Note::
            The motivation for using this function over `applyEventCuts` is that
            `applyEventCuts` completely removes events from processing. If we combine
            multiple skims in a single steering file (which is done in production), and
            the first has a set of event-level cuts, then all the remaining skims will
            never even see those events.

            Internally, this function creates a new path, which is only processed for
            events passing the event-level cut. To avoid issues around particles not
            being available on the main path (leading to noisy error logs), we need to
            add the rest of the skim to this path. So this new path is assigned to the
            attribute ``BaseSkim._ConditionalPath``, and ``BaseSkim.__call__`` will run
            all remaining methods on this path.
        """
        if self._ConditionalPath is not None:
            b2.B2FATAL(
                "BaseSkim.skim_event_cuts cannot be run twice in one skim. "
                "Please join your event-level cut strings into a single string."
            )

        ConditionalPath = b2.Path()
        self._ConditionalPath = ConditionalPath

        eselect = path.add_module("VariableToReturnValue", variable=f"passesEventCut({cut})")
        eselect.if_value('=1', ConditionalPath, b2.AfterConditionPath.CONTINUE)

        return ConditionalPath

    def get_skim_list_names(self):
        """
        Get the list of skim particle list names, without creating the particle lists on
        the current path.
        """
        DummyPath = b2.Path()

        OriginalSkimListsValue = self.SkimLists
        self.build_lists(DummyPath)
        SkimLists = self.SkimLists
        self.SkimLists = OriginalSkimListsValue

        return SkimLists

    def _method_unchanged(self, method):
        """Check if the method of the class is the same as in its parent class, or if it has
        been overridden.

        Useful for determining if *e.g.* `validation_histograms` has been defined for a
        particular skim.
        """
        cls = self.__class__
        ParentsWithAttr = [parent for parent in cls.__mro__[1:] if hasattr(parent, method)]

        if ParentsWithAttr:
            # Look for oldest ancestor which as that attribute, to handle inheritance.
            # In the case of `validation_histograms`, this will be `BaseSkim`.
            OldestParentWithAttr = ParentsWithAttr[-1]
            return getattr(cls, method) == getattr(OldestParentWithAttr, method)
        else:
            return False

    def __str__(self):
        return self.name

    def __name__(self):
        return self.name

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

    def apply_hlt_hadron_cut(self, path):
        """Apply the ``hlt_hadron`` selection if the property ``ApplyHLTHadronCut`` is True.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        hlt_hadron = "SoftwareTriggerResult(software_trigger_cut&skim&accept_hadron)"
        if self.ApplyHLTHadronCut:
            for SkimList in self.SkimLists:
                applyCuts(SkimList, f"{hlt_hadron}==1", path=path)


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

    When skims are combined using this class, the `BaseSkim.NoisyModules` lists of each
    skim are combined and all silenced.

    The heavy-lifting functions `BaseSkim.additional_setup`, `BaseSkim.build_lists` and
    `BaseSkim.output_udst` are modified to loop over the corresponding functions of each
    invididual skim. The `load_standard_lists` method is also modified to load all
    required lists, without accidentally loading a list twice.

    Calling an instance of the `CombinedSkim` class will load all the required particle
    lists, then run all the setup steps, then the list building functions, and then all
    the output steps.
    """

    __authors__ = ["Phil Grace"]
    __description__ = None
    __category__ = "combined"
    __contact__ = None

    def __init__(self, *skims, NoisyModules=None, CombinedSkimName="CombinedSkim"):
        if NoisyModules is None:
            NoisyModules = []
        # Check that we were passed only BaseSkim objects
        if not all([isinstance(skim, BaseSkim) for skim in skims]):
            raise NotImplementedError(
                "Must pass only `BaseSkim` type objects to `CombinedSkim`."
            )

        self.Skims = skims
        self.name = CombinedSkimName
        self.NoisyModules = list({mod for skim in skims for mod in skim.NoisyModules}) + NoisyModules
        self.TestFiles = list({f for skim in skims for f in skim.TestFiles})

        self.merge_data_structures()

    def __str__(self):
        return self.name

    def __name__(self):
        self.name

    def __call__(self, path):
        self.set_skim_logging(path)
        self.load_standard_lists(path)
        self.additional_setup(path)
        self.build_lists(path)
        self.apply_hlt_hadron_cut(path)
        self._check_duplicate_list_names()
        self.output_udst(path)

    def load_standard_lists(self, path):
        """Add all required standard list loading to the path.

        Note:
            To avoid loading standard lists twice, this function creates dummy paths
            that are passed through ``load_standard_lists`` for each skim. These dummy
            paths are then inspected, and a list of unique module-parameter combinations
            is added to the main skim path.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        ModulesAndParams = []
        for skim in self.Skims:
            DummyPath = b2.Path()
            skim.load_standard_lists(DummyPath)

            # Create a hashable data object to store the information about which
            # standard lists have been added to the path.
            ModulesAndParams.extend(tuple([
                (
                    module.type(),
                    tuple(sorted(
                        (param.name, param.values[0] if isinstance(param.values, list) else param.values)
                        for param in module.available_params()
                        if param.values != param.default
                    )),
                )
                for module in DummyPath.modules()
            ]))

        # Take this data structure and convert it to a dict. This removes any duplicate entries.
        ModulesAndParams = dict.fromkeys(ModulesAndParams)

        # Add the (now unique) module+param combinations to the main path
        for module, params in ModulesAndParams:
            path.add_module(module, **dict(params))

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
            skim.build_lists(skim._ConditionalPath or path)

    def output_udst(self, path):
        """Run the `BaseSkim.output_udst` function of each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self.Skims:
            skim.output_udst(skim._ConditionalPath or path)

    def apply_hlt_hadron_cut(self, path):
        """Run the `BaseSkim.apply_hlt_hadron_cut` function for each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self.Skims:
            skim.apply_hlt_hadron_cut(skim._ConditionalPath or path)

    def merge_data_structures(self):
        """Read the values of `BaseSkim.MergeDataStructures` and merge data structures
        accordingly.

        For example, if ``MergeDataStructures`` has the value ``{"FEIChannelArgs":
        _merge_boolean_dicts.__func__}``, then ``_merge_boolean_dicts`` is run on all
        input skims with the attribute ``FEIChannelArgs``, and the value of
        ``FEIChannelArgs`` for that skim is set to the result.

        In the FEI skims, this is used to merge configs which are passed to a cached
        function, thus allowing us to apply the FEI once with all the required particles
        available.
        """
        for iSkim, skim in enumerate(self.Skims):
            for attribute, MergingFunction in skim.MergeDataStructures.items():
                SkimsWithAttribute = [skim for skim in self.Skims if hasattr(skim, attribute)]
                setattr(
                    self.Skims[iSkim],
                    attribute,
                    MergingFunction(*[getattr(skim, attribute) for skim in SkimsWithAttribute])
                )

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
                f"{', '.join(DuplicatedParticleLists)}"
            )
