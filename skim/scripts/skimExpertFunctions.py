#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from abc import ABC, abstractmethod
import subprocess
import json
from pathlib import Path
import re

import yaml

import basf2 as b2
from modularAnalysis import applyCuts, summaryOfLists
from skim.registry import Registry


def get_test_file(sampleName):
    """
    Return the KEKCC location of files used specifically for skim testing

    Args:
        sampleName (str): Name of the sample. MC samples are named *e.g.* "MC12_chargedBGx1", "MC9_ccbarBGx0"

    Returns:
        The path to the test file on KEKCC.
    """

    with open(b2.find_file("skim/scripts/TestFiles.yaml")) as f:
        skimTestFilesInfo = yaml.safe_load(f)

    try:
        return skimTestFilesInfo[sampleName]
    except KeyError:
        msg = f"Sample {sampleName} not listed in skim/scripts/TestFiles.yaml."
        b2.B2ERROR(msg)
        raise KeyError(msg)


def get_file_metadata(filename):
    """
    Retrieve the metadata for a file using ``b2file-metadata-show``.

    Parameters:
       metadata (str): File to get number of events from.

    Returns:
        metadata: Metadata of file in dict format.
    """
    if not Path(filename).exists():
        raise FileNotFoundError(f"Could not find file {filename}")

    proc = subprocess.run(
        ["b2file-metadata-show", "--json", str(filename)],
        stdout=subprocess.PIPE,
        check=True,
    )
    metadata = json.loads(proc.stdout.decode("utf-8"))
    return metadata


def get_eventN(filename):
    """
    Retrieve the number of events in a file using ``b2file-metadata-show``.

    Parameters:
       filename (str): File to get number of events from.

    Returns:
        nEvents: Number of events in the file.
    """
    return int(get_file_metadata(filename)["nEvents"])


def resolve_skim_modules(SkimsOrModules, *, LocalModule=None):
    """
    Produce an ordered list of skims, by expanding any Python skim module names into a
    list of skims in that module. Also produce a dict of skims grouped by Python module.

    Raises:
        RuntimeError: Raised if a skim is listed twice.
        ValueError: Raised if ``LocalModule`` is passed and skims are normally expected
            from more than one module.
    """
    skims = []

    for name in SkimsOrModules:
        if name in Registry.names:
            skims.append(name)
        elif name in Registry.modules:
            skims.extend(Registry.get_skims_in_module(name))

    duplicates = set([skim for skim in skims if skims.count(skim) > 1])
    if duplicates:
        raise RuntimeError(
            f"Skim{'s'*(len(duplicates)>1)} requested more than once: {', '.join(duplicates)}"
        )

    modules = sorted({Registry.get_skim_module(skim) for skim in skims})
    if LocalModule:
        if len(modules) > 1:
            raise ValueError(
                f"Local module {LocalModule} specified, but the combined skim expects "
                "skims from more than one module. No steering file written."
            )
        modules = {LocalModule.rstrip(".py"): sorted(skims)}
    else:
        modules = {
            f"skim.{module}": sorted(
                [skim for skim in skims if Registry.get_skim_module(skim) == module]
            )
            for module in modules
        }

    return skims, modules


class InitialiseSkimFlag(b2.Module):
    """
    *[Module for skim expert usage]* Create the EventExtraInfo DataStore object, and set
    all required flag variables to zero.

    .. Note::

        Add this module to the path before adding any skims, so that the skim flags are
        defined in the datastore for all events.
    """

    def __init__(self, *skims):
        """
        Initialise module.

        Parameters:
            skims (skimExpertFunctions.BaseSkim): Skim to initialise event flag for.
        """

        from variables import variables as vm
        from ROOT import Belle2

        super().__init__()
        self.skims = skims
        self.EventExtraInfo = Belle2.PyStoreObj("EventExtraInfo")

        # Create aliases for convenience
        for skim in skims:
            vm.addAlias(skim.flag, f"eventExtraInfo({skim.flag})")

    def initialize(self):
        """
        Register EventExtraInfo in datastore if it has not been registered already.
        """
        if not self.EventExtraInfo.isValid():
            self.EventExtraInfo.registerInDataStore()

    def event(self):
        """
        Initialise flags to zero.
        """

        self.EventExtraInfo.create()
        for skim in self.skims:
            self.EventExtraInfo.addExtraInfo(skim.flag, 0)


class UpdateSkimFlag(b2.Module):
    """
    *[Module for skim expert usage]* Update the skim flag to be 1 if there is at least
    one candidate in any of the skim lists.

    .. Note::

        Add this module to the post-skim path of each skim in the combined skim, as the
        skim lists are only guaranteed to exist on the conditional path (if a
        conditional path was used).
    """

    def __init__(self, skim):
        """
        Initialise module.

        Parameters:
            skim (skimExpertFunctions.BaseSkim): Skim to update event flag for.
        """

        from ROOT import Belle2

        super().__init__()
        self.skim = skim
        self.EventExtraInfo = Belle2.PyStoreObj("EventExtraInfo")

    def initialize(self):
        """
        Check EventExtraInfo has been registered previously. This registration should be
        done by InitialiseSkimFlag.
        """
        self.EventExtraInfo.isRequired()

    def event(self):
        """
        Check if at least one skim list is non-empty; if so, update the skim flag to 1.
        """

        from ROOT import Belle2

        ListObjects = [Belle2.PyStoreObj(lst) for lst in self.skim.SkimLists]

        # Check required skim lists have been built on this path
        if any([not ListObj.isValid() for ListObj in ListObjects]):
            b2.B2FATAL(
                f"Error in UpdateSkimFlag for {self.skim}: particle lists not built. "
                "Did you add this module to the pre-skim path rather than the post-skim path?"
            )

        nCandidates = sum(ListObj.getListSize() for ListObj in ListObjects)

        # Override ExtraInfo flag if at least one candidate from any list passed
        if nCandidates > 0:
            self.EventExtraInfo.setExtraInfo(self.skim.flag, 1)


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

    produce_on_tau_samples = True
    """If this property is set to False, then ``b2skim-prod`` will not produce data
    production requests for this skim on taupair MC samples. This decision may be made
    for one of two reasons:

    * The retention rate of the skim on taupair samples is basically zero, so there is
      no point producing the skim for these samples.

    * The retention rate of the skim on taupair samples is too high (>20%), so the
      production system may struggle to handle the jobs.
    """

    validation_sample = None
    """
    MDST sample to use for validation histograms. Must be a valid location of a
    validation dataset (see documentation for `basf2.find_file`).
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

    @property
    def code(self):
        """Eight-digit code assigned to this skim in the registry."""
        return Registry.encode_skim_name(self.name)

    def __init__(self, *, OutputFileName=None, additionalDataDescription=None, udstOutput=True, validation=False):
        """Initialise the BaseSkim class.

        Parameters:
            OutputFileName (str): Name to give output uDST files. If none given, then
                defaults to eight-number skim code.
            additionalDataDescription (dict): additional data description to be added to the output file metadata.
            udstOutput (bool): If True, add uDST output to the path.
            validation (bool): If True, build lists and write validation histograms
                instead of writing uDSTs.
        """
        self.name = self.__class__.__name__
        self.OutputFileName = OutputFileName
        self.additionalDataDescription = additionalDataDescription
        self._udstOutput = udstOutput
        self._validation = validation
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
        Perform any setup steps necessary before running the skim.

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
    def __call__(self, path, *, udstOutput=None, validation=None):
        """Produce the skim particle lists and write uDST file.

        Parameters:
            path (basf2.Path): Skim path to be processed.
            udstOutput (bool): [DEPRECATED ARGUMENT] If True, add uDST output to the path.
            validation (bool): [DEPRECATED ARGUMENT] If True, build lists and write
                validation histograms instead of writing uDSTs.
        """
        # Deprecation warning. All configuration should be done during initialisation.
        warning = (
            "Passing the `{arg}` argument to `BaseSkim.__call__` is deprecated. "
            "Please pass all configuration parameters to the initialisation of "
            "the skim object."
        )
        if udstOutput is not None:
            b2.B2WARNING(warning.format(arg="udstOutput"))
            self._udstOutput = udstOutput
        if validation is not None:
            b2.B2WARNING(warning.format(arg="validation"))
            self._validation = validation

        self._MainPath = path

        self.set_skim_logging(path)
        self.initialise_skim_flag(path)
        self.load_standard_lists(path)
        self.additional_setup(path)
        # At this point, BaseSkim.skim_event_cuts may have been run, so pass
        # self._ConditionalPath for the path if it is not None (otherwise just pass the
        # regular path)
        self.build_lists(self._ConditionalPath or path)
        self.apply_hlt_hadron_cut_if_required(self._ConditionalPath or path)

        self.update_skim_flag(self._ConditionalPath or path)

        if self._udstOutput:
            self.output_udst(self._ConditionalPath or path)

        if self._validation:
            if self._method_unchanged("validation_histograms"):
                b2.B2FATAL(f"No validation histograms defined for {self} skim.")
            self.validation_histograms(self._ConditionalPath or path)

    @property
    def postskim_path(self):
        """
        Return the skim path.

        * If `BaseSkim.skim_event_cuts` has been run, then the skim lists will only be
          created on a conditional path, so subsequent modules should be added to the
          conditional path.

        * If `BaseSkim.skim_event_cuts` has not been run, then the main analysis path is
          returned.
        """

        if not self._MainPath:
            raise ValueError("Skim has not been added to the path yet!")
        return self._ConditionalPath or self._MainPath

    _MainPath = None
    """Main analysis path."""

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

    @property
    def flag(self):
        """
        Event-level variable indicating whether an event passes the skim or not. To use
        the skim flag without writing uDST output, use the argument ``udstOutput=False``
        when instantiating the skim class.
        """
        return f"passes_{self}"

    def initialise_skim_flag(self, path):
        """
        Add the module `skimExpertFunctions.InitialiseSkimFlag` to the path, which
        initialises flag for this skim to zero.
        """
        path.add_module(InitialiseSkimFlag(self))

    def update_skim_flag(self, path):
        """
        Add the module `skimExpertFunctions.InitialiseSkimFlag` to the path, which
        initialises flag for this skim to zero.

        .. Warning::

            If a conditional path has been created before this, then this function
            *must* run on the conditional path, since the skim lists are not guaranteed
            to exist for all events on the main path.
        """
        path.add_module(UpdateSkimFlag(self))

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

        # Keep this import here to avoid ROOT hijacking the argument parser
        import udst  # noqa

        # Make a fuss if self.SkimLists is empty
        if len(self.SkimLists) == 0:
            b2.B2FATAL(
                f"No skim list names defined in self.SkimLists for {self} skim!"
            )

        udst.add_skimmed_udst_output(
            skimDecayMode=self.code,
            skimParticleLists=self.SkimLists,
            outputFile=self.OutputFileName,
            dataDescription=self.additionalDataDescription,
            path=path,
        )
        summaryOfLists(self.SkimLists, path=path)

    def apply_hlt_hadron_cut_if_required(self, path):
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

    def __init__(
            self,
            *skims,
            NoisyModules=None,
            additionalDataDescription=None,
            udstOutput=None,
            mdstOutput=False,
            mdst_kwargs=None,
            CombinedSkimName="CombinedSkim",
            OutputFileName=None,
    ):
        """Initialise the CombinedSkim class.

        Parameters:
            *skims (BaseSkim): One or more (instantiated) skim objects.
            NoisyModules (list(str)): Additional modules to silence.
            additionalDataDescription (dict): Overrides corresponding setting of all individual skims.
            udstOutput (bool): Overrides corresponding setting of all individual skims.
            mdstOutput (bool): Write a single MDST output file containing events which
                pass any of the skims in this combined skim.
            mdst_kwargs (dict): kwargs to be passed to `mdst.add_mdst_output`. Only used
                if ``mdstOutput`` is True.
            CombinedSkimName (str): Sets output of ``__str__`` method of this combined skim.
            OutputFileName (str): If mdstOutput=True, this option sets the name of the combined output file.
                If mdstOutput=False, this option does nothing.
        """

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

        # empty but needed for functions inherited from baseSkim to work
        self.SkimLists = []

        if additionalDataDescription is not None:
            for skim in self:
                skim.additionalDataDescription = additionalDataDescription

        self._udstOutput = udstOutput
        if udstOutput is not None:
            for skim in self:
                skim._udstOutput = udstOutput

        self._mdstOutput = mdstOutput
        self.mdst_kwargs = mdst_kwargs or {}
        self.mdst_kwargs.update(OutputFileName=OutputFileName)

        self.merge_data_structures()

    def __str__(self):
        return self.name

    def __name__(self):
        return self.name

    def __call__(self, path):
        for skim in self:
            skim._MainPath = path

        self.set_skim_logging(path)
        self.initialise_skim_flag(path)
        self.load_standard_lists(path)
        self.additional_setup(path)
        self.build_lists(path)
        self.apply_hlt_hadron_cut_if_required(path)
        self.update_skim_flag(path)
        self._check_duplicate_list_names()
        self.output_udst(path)
        self.output_mdst_if_any_flag_passes(path=path, **self.mdst_kwargs)

    def __iter__(self):
        yield from self.Skims

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
        for skim in self:
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
        for skim in self:
            skim.additional_setup(path)

    def build_lists(self, path):
        """Run the `BaseSkim.build_lists` function of each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self:
            skim.build_lists(skim._ConditionalPath or path)

    def output_udst(self, path):
        """Run the `BaseSkim.output_udst` function of each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self:
            if skim._udstOutput:
                skim.output_udst(skim._ConditionalPath or path)

    def output_mdst_if_any_flag_passes(self, *, path, **kwargs):
        """
        Add MDST output to the path if the event passes any of the skim flags.
        EventExtraInfo is included in the MDST output so that the flags are available in
        the output.

        The ``CombinedSkimName`` parameter in the `CombinedSkim` initialisation is used
        for the output filename if ``filename`` is not included in kwargs.

        Parameters:
            path (basf2.Path): Skim path to be processed.
            **kwargs: Passed on to `mdst.add_mdst_output`.
        """
        from mdst import add_mdst_output

        if not self._mdstOutput:
            return

        sum_flags = " + ".join(f"eventExtraInfo({f})" for f in self.flags)
        variable = f"formula({sum_flags})"

        passes_flag_path = b2.Path()
        passes_flag = path.add_module("VariableToReturnValue", variable=variable)
        passes_flag.if_value(">0", passes_flag_path, b2.AfterConditionPath.CONTINUE)

        filename = kwargs.get("filename", kwargs.get("OutputFileName", self.name))

        if filename is None:
            filename = self.name

        if not filename.endswith(".mdst.root"):
            filename += ".mdst.root"

        kwargs["filename"] = filename

        if "OutputFileName" in kwargs.keys():
            del kwargs["OutputFileName"]

        kwargs.setdefault("dataDescription", {})

        # If the combinedSkim is not in the registry getting the code will throw a LookupError.
        # There is no requirement that a combinedSkim with single MDST output is
        # registered so set the skimDecayMode to ``None`` if no code is defined.
        try:
            skim_code = self.code
        except LookupError:
            skim_code = None
        kwargs["dataDescription"].setdefault("skimDecayMode", skim_code)

        try:
            kwargs["additionalBranches"] += ["EventExtraInfo"]
        except KeyError:
            kwargs["additionalBranches"] = ["EventExtraInfo"]

        add_mdst_output(path=passes_flag_path, **kwargs)

    def apply_hlt_hadron_cut_if_required(self, path):
        """Run the `BaseSkim.apply_hlt_hadron_cut_if_required` function for each skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.
        """
        for skim in self:
            skim.apply_hlt_hadron_cut_if_required(skim._ConditionalPath or path)

    @property
    def flags(self):
        """
        List of flags for each skim in combined skim.
        """
        return [skim.flag for skim in self]

    @property
    def flag(self):
        """
        Event-level variable indicating whether an event passes the combinedSkim or not.
        """
        return f"passes_{self}"

    def initialise_skim_flag(self, path):
        """
        Add the module `skimExpertFunctions.InitialiseSkimFlag` to the path, to
        initialise flags for each skim.
        """
        path.add_module(InitialiseSkimFlag(*self))

    def update_skim_flag(self, path):
        """
        Add the module `skimExpertFunctions.InitialiseSkimFlag` to the conditional path
        of each skims.
        """
        for skim in self:
            skim.postskim_path.add_module(UpdateSkimFlag(skim))

    @property
    def produce_on_tau_samples(self):
        """
        Corresponding value of this attribute for each individual skim.

        Raises:
            RuntimeError: Raised if the individual skims in combined skim contain a mix
                of True and False for this property.
        """
        produce_on_tau = [skim.produce_on_tau_samples for skim in self]
        if all(produce_on_tau):
            return True
        elif all(not TauBool for TauBool in produce_on_tau):
            return False
        else:
            raise RuntimeError(
                "The individual skims in the combined skim contain a mix of True and "
                "False for the attribute `produce_on_tau_samples`.\n"
                "    It is unclear what should be done in this situation."
                "Please reorganise the combined skims to address this.\n"
                "    Skims included in the problematic combined skim: "
                f"{', '.join(skim.name for skim in self)}"
            )

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
                SkimsWithAttribute = [skim for skim in self if hasattr(skim, attribute)]
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
        ParticleListLists = [skim.SkimLists for skim in self]
        ParticleLists = [lst for L in ParticleListLists for lst in L]
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
