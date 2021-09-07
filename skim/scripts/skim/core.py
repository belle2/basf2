#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
The core classes of the skim package are defined in ``skim.core``: ``BaseSkim`` and
``CombinedSkim``.

* ``BaseSkim`` is an abstract base class from which all skims inherit. It defines
  template functions for a skim, and includes attributes describing the skim metadata.

* ``CombinedSkim`` is a class for combining ``BaseSkim`` objects into a single steering
  file.
"""

from abc import ABC, abstractmethod
import warnings

import basf2 as b2
from modularAnalysis import applyCuts, summaryOfLists
from skim.registry import Registry
from skim.utils.flags import InitialiseSkimFlag, UpdateSkimFlag
from skim.utils.testfiles import get_test_file


class BaseSkim(ABC):
    """Base class for skims. Initialises a skim name, and creates template functions
    required for each skim.

    See `writing-skims` for information on how to use this to define a new skim.
    """

    NoisyModules = None
    """List of module types to be silenced. This may be necessary in certain skims in
    order to keep log file sizes small.

    .. tip::
        The elements of this list should be the module *type*, which is not necessarily
        the same as the module name. The module type can be inspected in Python via
        ``module.type()``.

    .. seealso::
        This attribute is used by `BaseSkim.set_skim_logging`.
    """

    TestSampleProcess = "mixed"
    """MC process of test file. `BaseSkim.TestFiles` passes this property to
    `skim.utils.testfiles.get_test_file` to retrieve an appropriate file location.
    Defaults to a :math:`B^{0}\\overline{B^{0}}` sample.
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

    produces_mdst_by_default = False
    """Special property for combined systematics skims, which produce MDST output instead of
    uDST. This property is used by ``b2skim-prod`` to set the ``DataLevel`` parameter in
    the ``DataDescription`` block for this skim to ``mdst`` instead of ``udst``.
    """

    validation_sample = None
    """
    MDST sample to use for validation histograms. Must be a valid location of a
    validation dataset (see documentation for `basf2.find_file`).
    """

    mc = True
    """
    Include Monte Carlo quantities in skim output.
    """

    analysisGlobaltag = None
    """
    Analysis globaltag.
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

    def __init__(
        self,
        *,
        OutputFileName=None,
        additionalDataDescription=None,
        udstOutput=True,
        validation=False,
        mc=True,
        analysisGlobaltag=None,
    ):
        """Initialise the BaseSkim class.

        Parameters:
            OutputFileName (str): Name to give output uDST files. If none given, then
                defaults to eight-number skim code.
            additionalDataDescription (dict): additional data description to be added to the output file metadata.
            udstOutput (bool): If True, add uDST output to the path.
            validation (bool): If True, build lists and write validation histograms
                instead of writing uDSTs.
            mc (bool): If True, include MC quantities in output.
            analysisGlobaltag (str): Analysis globaltag.
        """
        self.name = self.__class__.__name__
        self.OutputFileName = OutputFileName
        self.additionalDataDescription = additionalDataDescription
        self._udstOutput = udstOutput
        self._validation = validation
        self.mc = mc
        self.analysisGlobaltag = analysisGlobaltag

        if self.NoisyModules is None:
            self.NoisyModules = []

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
        the main skim cuts should be applied. This function should return a list of
        particle list names.

        Parameters:
            path (basf2.Path): Skim path to be processed.

        .. versionchanged:: release-06-00-00

           Previously, this function was expected to set the attribute
           `BaseSkim.SkimLists`. Now this is handled by `BaseSkim`, and this function is
           expected to return the list of particle list names.
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
        self._MainPath = path

        self.initialise_skim_flag(path)
        self.load_standard_lists(path)
        self.additional_setup(path)
        # At this point, BaseSkim.skim_event_cuts may have been run, so pass
        # self._ConditionalPath for the path if it is not None (otherwise just pass the
        # regular path)
        self.SkimLists = self.build_lists(self._ConditionalPath or path)
        self.apply_hlt_hadron_cut_if_required(self._ConditionalPath or path)

        self.update_skim_flag(self._ConditionalPath or path)

        if self._udstOutput:
            self.output_udst(self._ConditionalPath or path)

        if self._validation:
            if self._method_unchanged("validation_histograms"):
                b2.B2FATAL(f"No validation histograms defined for {self} skim.")
            self.validation_histograms(self._ConditionalPath or path)

        self.set_skim_logging()

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

    SkimLists = []
    """
    List of particle lists reconstructed by the skim. This attribute should only be
    accessed after running the ``__call__`` method.
    """

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
    def TestFiles(self):
        """
        Location of test MDST sample. To modify this, set the property
        `BaseSkim.TestSampleProcess`, and this function will find an appropriate test
        sample from the list in
        ``/group/belle2/dataprod/MC/SkimTraining/SampleLists/TestFiles.yaml``

        If no sample can be found, an empty list is returned.
        """
        try:
            return [str(get_test_file(process=self.TestSampleProcess))]
        except FileNotFoundError:
            # Could not find TestFiles.yaml
            # (Don't issue a warning, since this will just show up as noise during grid processing)
            return []
        except ValueError:
            b2.B2WARNING(
                f"Could not find '{self.TestSampleProcess}' sample in TestFiles.yaml"
            )
            # Could not find sample in YAML file
            return []

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
        Add the module `skim.utils.flags.InitialiseSkimFlag` to the path, which
        initialises flag for this skim to zero.
        """
        path.add_module(InitialiseSkimFlag(self))

    def update_skim_flag(self, path):
        """
        Add the module `skim.utils.flags.UpdateSkimFlag` to the path, which
        updates flag for this skim.

        .. Warning::

            If a conditional path has been created before this, then this function
            *must* run on the conditional path, since the skim lists are not guaranteed
            to exist for all events on the main path.
        """
        path.add_module(UpdateSkimFlag(self))

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

    def set_skim_logging(self):
        """Turns the log level to ERROR for selected modules to decrease the total size
        of the skim log files. Additional modules can be silenced by setting the attribute
        `NoisyModules` for an individual skim.

        Parameters:
            path (basf2.Path): Skim path to be processed.

        .. warning::

            This method works by inspecting the modules added to the path, and setting
            the log level to ERROR. This method should be called *after* all
            skim-related modules are added to the path.
        """
        b2.set_log_level(b2.LogLevel.INFO)

        NoisyModules = ["ParticleLoader", "ParticleVertexFitter"] + self.NoisyModules

        # Set log level of modules on both main path and conditional path
        paths = filter(None, (self._MainPath, self._ConditionalPath))
        modules = [module for path in paths for module in path.modules()]

        for module in modules:
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
            mc=self.mc,
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
        from skim.WGs.foo import OneSkim, TwoSkim, RedSkim, BlueSkim

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
            mc=None,
            analysisGlobaltag=None,
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
            mc (bool): If True, include MC quantities in output.
            analysisGlobaltag (str): Analysis globaltag.
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
        for skim in self:
            skim.NoisyModules += NoisyModules

        # empty but needed for functions inherited from baseSkim to work
        self.SkimLists = []

        if additionalDataDescription is not None:
            for skim in self:
                skim.additionalDataDescription = additionalDataDescription

        self._udstOutput = udstOutput
        if udstOutput is not None:
            for skim in self:
                skim._udstOutput = udstOutput

        self.mc = mc
        if mc is not None:
            for skim in self:
                skim.mc = mc

        self.analysisGlobaltag = analysisGlobaltag
        if analysisGlobaltag is not None:
            for skim in self:
                skim.analysisGlobaltag = analysisGlobaltag

        self._mdstOutput = mdstOutput
        self.mdst_kwargs = mdst_kwargs or {}
        self.mdst_kwargs.update(OutputFileName=OutputFileName)

        if mc is not None:
            self.mdst_kwargs.update(mc=mc)

        self.merge_data_structures()

    def __str__(self):
        return self.name

    def __name__(self):
        return self.name

    def __call__(self, path):
        for skim in self:
            skim._MainPath = path

        self.initialise_skim_flag(path)
        self.load_standard_lists(path)
        self.additional_setup(path)
        self.build_lists(path)
        self.apply_hlt_hadron_cut_if_required(path)
        self.update_skim_flag(path)
        self._check_duplicate_list_names()
        self.output_udst(path)
        self.output_mdst_if_any_flag_passes(path=path, **self.mdst_kwargs)
        self.set_skim_logging()

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
            skim.SkimLists = skim.build_lists(skim._ConditionalPath or path)

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

        filename = kwargs.get("filename", kwargs.get("OutputFileName", self.code))

        if filename is None:
            filename = self.code

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

    def set_skim_logging(self):
        """Run `BaseSkim.set_skim_logging` for each skim."""
        for skim in self:
            skim.set_skim_logging()

    @property
    def TestFiles(self):
        return list({f for skim in self for f in skim.TestFiles})

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
        Add the module `skim.utils.flags.InitialiseSkimFlag` to the path, to
        initialise flags for each skim.
        """
        path.add_module(InitialiseSkimFlag(*self))

    def update_skim_flag(self, path):
        """
        Add the module `skim.utils.flags.UpdateSkimFlag` to the conditional path
        of each skims.
        """
        for skim in self:
            skim.postskim_path.add_module(UpdateSkimFlag(skim))

    @property
    def produce_on_tau_samples(self):
        """
        Corresponding value of this attribute for each individual skim.

        A warning is issued if the individual skims in combined skim contain a mix of
        True and False for this property.
        """
        produce_on_tau = [skim.produce_on_tau_samples for skim in self]
        if all(produce_on_tau):
            return True
        elif all(not TauBool for TauBool in produce_on_tau):
            return False
        else:
            warnings.warn(
                (
                    "The individual skims in the combined skim contain a mix of True and "
                    "False for the attribute `produce_on_tau_samples`.\n    The default in "
                    "this case is to allow the combined skim to be produced on tau samples.\n"
                    "    Skims included in the problematic combined skim: "
                    f"{', '.join(skim.name for skim in self)}"
                ),
                RuntimeWarning,
            )
            return True

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
