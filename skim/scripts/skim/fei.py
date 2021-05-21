#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
(Semi-)Leptonic Working Group Skims for missing energy modes that use the `FullEventInterpretation` (FEI) algorithm.
"""

__authors__ = [
    "Racha Cheaib",
    "Hannah Wakeling",
    "Phil Grace"
]

from functools import lru_cache, wraps

import basf2 as b2
import fei
import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, _sphinxify_decay, fancy_skim_header
from variables import variables as vm

__liaison__ = "Shanette De La Motte <shanette.delamotte@adelaide.edu.au>"
_VALIDATION_SAMPLE = "mdst14.root"


def _merge_boolean_dicts(*dicts):
    """Merge dicts of boolean, with `True` values taking precedence if values
    differ.

    This is a utility function for combining FEI configs. It acts in the following
    way:

        >>> d1 = {"neutralB": True, "chargedB": False, "hadronic": True}
        >>> d2 = {"chargedB": True, "semileptonic": True}
        >>> _merge_FEI_configs(d1, d2)
        {"chargedB": True, "hadronic": True, "neutralB": True, "semileptonic": True}

    Parameters:
        dicts (dict(str -> bool)): Any number of dicts of keyword-boolean pairs.

    Returns:
        merged (dict(str -> bool)): A single dict, containing all the keys of the
            input dicts.
    """
    keys = {k for d in dicts for k in d}
    occurances = {k: [d for d in dicts if k in d] for k in keys}
    merged = {k: any(d[k] for d in occurances[k]) for k in keys}

    # Sort the merged dict before returning
    merged = dict(sorted(merged.items()))

    return merged


def _get_fei_channel_names(particleName, **kwargs):
    """Create a list containing the decay strings of all decay channels available to a
    particle. Any keyword arguments are passed to `fei.get_default_channels`.

    This is a utility function for autogenerating FEI skim documentation.

    Args:
        particleName (str): the PDG name of a particle, e.g. ``'K+'``, ``'pi-'``, ``'D*0'``.
    """
    particleList = fei.get_default_channels(**kwargs)
    particleDict = {particle.name: particle for particle in particleList}

    try:
        particle = particleDict[particleName]
    except KeyError:
        print(f"Error! Couldn't find particle with name {particleName}")
        return []

    channels = [channel.decayString for channel in particle.channels]
    return channels


def _hash_dict(func):
    """Wrapper for `functools.lru_cache` to deal with dictionaries. Dictionaries are
    mutable, so cannot be cached. This wrapper turns all dict arguments into a hashable
    dict type, so we can use caching.
    """
    class HashableDict(dict):
        def __hash__(self):
            return hash(frozenset(self.items()))

    @wraps(func)
    def wrapped(*args, **kwargs):
        args = tuple([HashableDict(arg) if isinstance(arg, dict) else arg for arg in args])
        kwargs = {k: HashableDict(v) if isinstance(v, dict) else v for k, v in kwargs.items()}
        return func(*args, **kwargs)
    return wrapped


class BaseFEISkim(BaseSkim):
    """Base class for FEI skims. Applies event-level pre-cuts and applies the FEI."""

    __authors__ = ["Racha Cheaib", "Hannah Wakeling", "Phil Grace"]
    __contact__ = __liaison__
    __category__ = "physics, Full Event Interpretation"

    FEIPrefix = "FEIv4_2021_MC14_release_05_01_12"
    """Prefix label for the FEI training used in the FEI skims."""

    FEIChannelArgs = {}
    """Dict of ``str -> bool`` pairs to be passed to `fei.get_default_channels`. When
    inheriting from `BaseFEISkim`, override this value to apply the FEI for only *e.g.*
    SL charged :math:`B`'s."""

    MergeDataStructures = {"FEIChannelArgs": _merge_boolean_dicts}

    NoisyModules = ["ParticleCombiner"]

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair

    @staticmethod
    @lru_cache()
    def fei_precuts(path):
        """
        Skim pre-cuts are applied before running the FEI, to reduce computation time.
        This setup function is run by all FEI skims, so they all have the save
        event-level pre-cuts:

        * :math:`n_{\\text{cleaned tracks}} \\geq 3`
        * :math:`n_{\\text{cleaned ECL clusters}} \\geq 3`
        * :math:`\\text{Visible energy of event (CMS frame)}>4~{\\rm GeV}`
        * :math:`2~{\\rm GeV}<E_{\\text{cleaned tracks & clusters in
          ECL}}<7~{\\rm GeV}`

        We define "cleaned" tracks and clusters as:

        * Cleaned tracks (``pi+:FEI_cleaned``): :math:`d_0 < 0.5~{\\rm cm}`,
          :math:`|z_0| < 2~{\\rm cm}`, and :math:`p_T > 0.1~{\\rm GeV}` * Cleaned ECL
          clusters (``gamma:FEI_cleaned``): :math:`0.296706 < \\theta < 2.61799`, and
          :math:`E>0.1~{\\rm GeV}`
        """

        # Pre-selection cuts
        CleanedTrackCuts = "abs(z0) < 2.0 and abs(d0) < 0.5 and pt > 0.1"
        CleanedClusterCuts = "E > 0.1 and 0.296706 < theta < 2.61799"

        ma.fillParticleList(decayString="pi+:FEI_cleaned",
                            cut=CleanedTrackCuts, path=path)
        ma.fillParticleList(decayString="gamma:FEI_cleaned",
                            cut=CleanedClusterCuts, path=path)

        ma.buildEventKinematics(inputListNames=["pi+:FEI_cleaned",
                                                "gamma:FEI_cleaned"],
                                path=path)

        EventCuts = " and ".join(
            [
                f"nCleanedTracks({CleanedTrackCuts})>=3",
                f"nCleanedECLClusters({CleanedClusterCuts})>=3",
                "visibleEnergyOfEventCMS>4",
                "2<E_ECL_FEI<7",
            ]
        )

        # NOTE: The FEI skims are somewhat complicated, and require some manual handling
        # of conditional paths to avoid adding the FEI to the path twice. In general, DO
        # NOT do this kind of path handling in your own skim. Instead, use:
        #     >>>  path = self.skim_event_cuts(EventLevelCuts, path=path)
        ConditionalPath = b2.Path()
        eselect = path.add_module("VariableToReturnValue", variable=f"passesEventCut({EventCuts})")
        eselect.if_value('=1', ConditionalPath, b2.AfterConditionPath.CONTINUE)

        return ConditionalPath

    # This is a cached static method so that we can avoid adding FEI path twice.
    # In combined skims, FEIChannelArgs must be combined across skims first, so that all
    # the required particles are included in the FEI.
    @staticmethod
    @_hash_dict
    @lru_cache()
    def run_fei_for_skims(FEIChannelArgs, FEIPrefix, *, path):
        """Reconstruct hadronic and semileptonic :math:`B^0` and :math:`B^+` tags using
        the generically trained FEI.

        Parameters:
            FEIChannelArgs (dict(str, bool)): A dict of keyword-boolean pairs to be
                passed to `fei.get_default_channels`.
            FEIPrefix (str): Prefix label for the FEI training used in the FEI skims.
            path (`basf2.Path`): The skim path to be processed.
        """
        # Run FEI
        b2.conditions.prepend_globaltag("analysis_tools_light-2104-poseidon")  # ma.getAnalysisGlobaltag())
        particles = fei.get_default_channels(**FEIChannelArgs)
        configuration = fei.config.FeiConfiguration(
            prefix=FEIPrefix,
            training=False,
            monitor=False)
        feistate = fei.get_path(particles, configuration)
        path.add_path(feistate.path)

    @staticmethod
    @_hash_dict
    @lru_cache()
    def setup_fei_aliases(FEIChannelArgs):
        # Aliases for pre-FEI event-level cuts
        vm.addAlias("E_ECL_pi_FEI",
                    "totalECLEnergyOfParticlesInList(pi+:FEI_cleaned)")
        vm.addAlias("E_ECL_gamma_FEI",
                    "totalECLEnergyOfParticlesInList(gamma:FEI_cleaned)")
        vm.addAlias("E_ECL_FEI", "formula(E_ECL_pi_FEI+E_ECL_gamma_FEI)")

        # Aliases for variables available after running the FEI
        vm.addAlias("sigProb", "extraInfo(SignalProbability)")
        vm.addAlias("log10_sigProb", "log10(extraInfo(SignalProbability))")
        vm.addAlias("dmID", "extraInfo(decayModeID)")
        vm.addAlias("decayModeID", "extraInfo(decayModeID)")

        if "semileptonic" in FEIChannelArgs and FEIChannelArgs["semileptonic"]:
            # Aliases specific to SL FEI
            vm.addAlias("cosThetaBY", "cosThetaBetweenParticleAndNominalB")
            vm.addAlias("d1_p_CMSframe", "useCMSFrame(daughter(1,p))")
            vm.addAlias("d2_p_CMSframe", "useCMSFrame(daughter(2,p))")
            vm.addAlias(
                "p_lepton_CMSframe",
                "conditionalVariableSelector(dmID<4, d1_p_CMSframe, d2_p_CMSframe)"
            )

    def additional_setup(self, path):
        """Apply pre-FEI event-level cuts and apply the FEI. This setup function is run
        by all FEI skims, so they all have the save event-level pre-cuts.

        This function passes `FEIChannelArgs` to the cached function `run_fei_for_skims`
        to avoid applying the FEI twice.

        See also:
            `fei_precuts` for event-level cut definitions.
        """
        self.setup_fei_aliases(self.FEIChannelArgs)
        path = self.fei_precuts(path)
        # The FEI skims require some manual handling of paths that is not necessary in
        # any other skim.
        self._ConditionalPath = path

        self.run_fei_for_skims(self.FEIChannelArgs, self.FEIPrefix, path=path)


def _FEI_skim_header(ParticleNames):
    """Decorator factory for applying the `fancy_skim_header` header and replacing
    <CHANNELS> in the class docstring with a list of FEI channels.

    The list is numbered with all of the corresponding decay mode IDs, and the decay
    modes are formatted in beautiful LaTeX.

    .. code-block:: python

        @FEI_skim_header("B0")
        class feiSLB0(BaseFEISkim):
            # docstring here including the string '<CHANNELS>' somewhere

    Parameters:
        ParticleNames (str, list(str)): One of either ``B0`` or ``B+``, or a list of both.
    """

    def decorator(SkimClass):
        if isinstance(ParticleNames, str):
            particles = [ParticleNames]
        else:
            particles = ParticleNames

        ChannelsString = "List of reconstructed channels and corresponding decay mode IDs:"
        for particle in particles:
            channels = _get_fei_channel_names(particle, **SkimClass.FEIChannelArgs)
            FormattedChannels = [_sphinxify_decay(channel) for channel in channels]
            ChannelList = "\n".join(
                [f"    {dmID}. {channel}"
                 for (dmID, channel) in enumerate(FormattedChannels)]
            )
            if len(particles) == 1:
                ChannelsString += "\n\n" + ChannelList
            else:
                ChannelsString += f"\n\n    ``{particle}`` channels:\n\n" + ChannelList

        if SkimClass.__doc__ is None:
            return SkimClass
        else:
            SkimClass.__doc__ = SkimClass.__doc__.replace("<CHANNELS>", ChannelsString)

        return fancy_skim_header(SkimClass)

    return decorator


@_FEI_skim_header("B0")
class feiHadronicB0(BaseFEISkim):
    """
    Tag side :math:`B` cuts:

    * :math:`M_{\\text{bc}} > 5.24~{\\rm GeV}`
    * :math:`|\\Delta E| < 0.2~{\\rm GeV}`
    * :math:`\\text{signal probability} > 0.001` (omitted for decay mode 23)

    All available FEI :math:`B^0` hadronic tags are reconstructed. From `Thomas Keck's
    thesis <https://docs.belle2.org/record/275/files/BELLE2-MTHESIS-2015-001.pdf>`_,
    "the channel :math:`B^0 \\to \\overline{D}^0 \\pi^0` was used by the FR, but is not
    yet used in the FEI due to unexpected technical restrictions in the KFitter
    algorithm".

    <CHANNELS>

    See also:
        `BaseFEISkim.FEIPrefix` for FEI training used, and `BaseFEISkim.fei_precuts` for
        event-level cuts made before applying the FEI.
    """
    __description__ = "FEI-tagged neutral :math:`B`'s decaying hadronically."
    validation_sample = _VALIDATION_SAMPLE

    FEIChannelArgs = {
        "neutralB": True,
        "chargedB": False,
        "hadronic": True,
        "semileptonic": False,
        "KLong": False,
        "baryonic": True
    }

    def build_lists(self, path):
        ma.applyCuts("B0:generic", "Mbc>5.24", path=path)
        ma.applyCuts("B0:generic", "abs(deltaE)<0.200", path=path)
        ma.applyCuts("B0:generic", "sigProb>0.001 or extraInfo(dmID)==23", path=path)

        self.SkimLists = ["B0:generic"]

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        vm.addAlias('sigProb', 'extraInfo(SignalProbability)')
        vm.addAlias('log10_sigProb', 'log10(extraInfo(SignalProbability))')
        vm.addAlias('d0_massDiff', 'daughter(0,massDifference(0))')
        vm.addAlias('d0_M', 'daughter(0,M)')
        vm.addAlias('decayModeID', 'extraInfo(decayModeID)')
        vm.addAlias('nDaug', 'countDaughters(1>0)')  # Dummy cut so all daughters are selected.

        histogramFilename = f"{self}_Validation.root"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist='B0:generic',
            variables_1d=[
                ('sigProb', 100, 0.0, 1.0, 'Signal probability', __liaison__,
                 'Signal probability of the reconstructed tag B candidates',
                 'Most around zero, with a tail at non-zero values.', 'Signal probability', 'Candidates', 'logy'),
                ('nDaug', 6, 0.0, 6, 'Number of daughters of tag B', __liaison__,
                 'Number of daughters of tag B', 'Some distribution of number of daughters', 'n_{daughters}', 'Candidates'),
                ('d0_massDiff', 100, 0.0, 0.5, 'Mass difference of D* and D', __liaison__,
                 'Mass difference of D^{*} and D', 'Peak at 0.14 GeV', 'm(D^{*})-m(D) [GeV]', 'Candidates', 'shifter'),
                ('d0_M', 100, 0.0, 3.0, 'Mass of zeroth daughter (D* or D)', __liaison__,
                 'Mass of zeroth daughter of tag B (either a $D^{*}$ or a D)', 'Peaks at 1.86 GeV and 2.00 GeV',
                 'm(D^{(*)}) [GeV]', 'Candidates', 'shifter'),
                ('deltaE', 40, -0.2, 0.2, '#Delta E', __liaison__,
                 '$\\Delta E$ of event', 'Peak around zero', '#Delta E [GeV]', 'Candidates', 'shifter'),
                ('Mbc', 40, 5.2, 5.3, 'Mbc', __liaison__,
                 'Beam-constrained mass of event', 'Peaking around B mass (5.28 GeV)', 'M_{bc} [GeV]', 'Candidates', 'shifter')],
            variables_2d=[('deltaE', 100, -0.2, 0.2, 'Mbc', 100, 5.2, 5.3, 'Mbc vs deltaE', __liaison__,
                           'Plot of the $\\Delta E$ of the event against the beam constrained mass',
                           'Peak of $\\Delta E$ around zero, and $M_{bc}$ around B mass (5.28 GeV)',
                           '#Delta E [GeV]', 'M_{bc} [GeV]', 'colz'),
                          ('decayModeID', 26, 0, 26, 'log10_sigProb', 100, -3.0, 0.0,
                           'Signal probability for each decay mode ID', __liaison__,
                           'Signal probability for each decay mode ID',
                           'Some distribtuion of candidates in the first few decay mode IDs',
                           'Decay mode ID', '#log_10(signal probability)', 'colz')],
            path=path)


@_FEI_skim_header("B+")
class feiHadronicBplus(BaseFEISkim):
    """
    Tag side :math:`B` cuts:

    * :math:`M_{\\text{bc}} > 5.24~{\\rm GeV}`
    * :math:`|\\Delta E| < 0.2~{\\rm GeV}`
    * :math:`\\text{signal probability} > 0.001` (omitted for decay mode 25)

    All available FEI :math:`B^+` hadronic tags are reconstructed.

    <CHANNELS>

    See also:
        `BaseFEISkim.FEIPrefix` for FEI training used, and `BaseFEISkim.fei_precuts` for
        event-level cuts made before applying the FEI.
    """
    __description__ = "FEI-tagged charged :math:`B`'s decaying hadronically."
    validation_sample = _VALIDATION_SAMPLE

    FEIChannelArgs = {
        "neutralB": False,
        "chargedB": True,
        "hadronic": True,
        "semileptonic": False,
        "KLong": False,
        "baryonic": True
    }

    def build_lists(self, path):
        ma.applyCuts("B+:generic", "Mbc>5.24", path=path)
        ma.applyCuts("B+:generic", "abs(deltaE)<0.200", path=path)
        ma.applyCuts("B+:generic", "sigProb>0.001 or extraInfo(dmID)==25", path=path)

        self.SkimLists = ["B+:generic"]

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        vm.addAlias('sigProb', 'extraInfo(SignalProbability)')
        vm.addAlias('log10_sigProb', 'log10(extraInfo(SignalProbability))')
        vm.addAlias('d0_massDiff', 'daughter(0,massDifference(0))')
        vm.addAlias('d0_M', 'daughter(0,M)')
        vm.addAlias('decayModeID', 'extraInfo(decayModeID)')
        vm.addAlias('nDaug', 'countDaughters(1>0)')  # Dummy cut so all daughters are selected.

        histogramFilename = f"{self}_Validation.root"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist='B+:generic',
            variables_1d=[
                ('sigProb', 100, 0.0, 1.0, 'Signal probability', __liaison__,
                 'Signal probability of the reconstructed tag B candidates', 'Most around zero, with a tail at non-zero values.',
                 'Signal probability', 'Candidates', 'logy'),
                ('nDaug', 6, 0.0, 6, 'Number of daughters of tag B', __liaison__,
                 'Number of daughters of tag B', 'Some distribution of number of daughters', 'n_{daughters}', 'Candidates'),
                ('d0_massDiff', 100, 0.0, 0.5, 'Mass difference of D* and D', __liaison__,
                 'Mass difference of D^{*} and D', 'Peak at 0.14 GeV', 'm(D^{*})-m(D) [GeV]', 'Candidates', 'shifter'),
                ('d0_M', 100, 0.0, 3.0, 'Mass of zeroth daughter (D* or D)', __liaison__,
                 'Mass of zeroth daughter of tag B (either a $D^{*}$ or a D)', 'Peaks at 1.86 GeV and 2.00 GeV',
                 'm(D^{(*)}) [GeV]', 'Candidates', 'shifter'),
                ('deltaE', 40, -0.2, 0.2, '#Delta E', __liaison__,
                 '$\\Delta E$ of event', 'Peak around zero', '#Delta E [GeV]', 'Candidates', 'shifter'),
                ('Mbc', 40, 5.2, 5.3, 'Mbc', __liaison__,
                 'Beam-constrained mass of event', 'Peak around B mass (5.28 GeV)', 'M_{bc} [GeV]', 'Candidates', 'shifter')],
            variables_2d=[('deltaE', 100, -0.2, 0.2, 'Mbc', 100, 5.2, 5.3, 'Mbc vs deltaE', __liaison__,
                           'Plot of the $\\Delta E$ of the event against the beam constrained mass',
                           'Peak of $\\Delta E$ around zero, and $M_{bc}$ around B mass (5.28 GeV)',
                           '#Delta E [GeV]', 'M_{bc} [GeV]', 'colz'),
                          ('decayModeID', 29, 0, 29, 'log10_sigProb', 100, -3.0, 0.0,
                           'Signal probability for each decay mode ID', __liaison__,
                           'Signal probability for each decay mode ID',
                           'Some distribtuion of candidates in the first few decay mode IDs',
                           'Decay mode ID', '#log_10(signal probability)', 'colz')],
            path=path)


@_FEI_skim_header("B0")
class feiSLB0(BaseFEISkim):
    """
    Tag side :math:`B` cuts:

    * :math:`-4 < \\cos\\theta_{BY} < 3`
    * :math:`\\log_{10}(\\text{signal probability}) > -2.4`
    * :math:`p_{\\ell}^{*} > 1.0~{\\rm GeV}` in CMS frame

    SL :math:`B^0` tags are reconstructed. Hadronic :math:`B` with SL :math:`D` are not
    reconstructed, as these are rare and time-intensive.

    <CHANNELS>

    See also:
        `BaseFEISkim.FEIPrefix` for FEI training used, and `BaseFEISkim.fei_precuts` for
        event-level cuts made before applying the FEI.
    """
    __description__ = "FEI-tagged neutral :math:`B`'s decaying semileptonically."
    validation_sample = _VALIDATION_SAMPLE

    FEIChannelArgs = {
        "neutralB": True,
        "chargedB": False,
        "hadronic": False,
        "semileptonic": True,
        "KLong": False,
        "baryonic": True,
        "removeSLD": True
    }

    def build_lists(self, path):
        ma.applyCuts("B0:semileptonic", "dmID<8", path=path)
        ma.applyCuts("B0:semileptonic", "log10(sigProb)>-2.4", path=path)
        ma.applyCuts("B0:semileptonic", "-4.0<cosThetaBY<3.0", path=path)
        ma.applyCuts("B0:semileptonic", "p_lepton_CMSframe>1.0", path=path)

        self.SkimLists = ["B0:semileptonic"]

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        vm.addAlias('sigProb', 'extraInfo(SignalProbability)')
        vm.addAlias('log10_sigProb', 'log10(extraInfo(SignalProbability))')
        vm.addAlias('d0_massDiff', 'daughter(0,massDifference(0))')
        vm.addAlias('d0_M', 'daughter(0,M)')
        vm.addAlias('decayModeID', 'extraInfo(decayModeID)')
        vm.addAlias('nDaug', 'countDaughters(1>0)')  # Dummy cut so all daughters are selected.

        histogramFilename = f"{self}_Validation.root"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist='B0:semileptonic',
            variables_1d=[
                ('sigProb', 100, 0.0, 1.0, 'Signal probability', __liaison__,
                 'Signal probability of the reconstructed tag B candidates', 'Most around zero, with a tail at non-zero values.',
                 'Signal probability', 'Candidates', 'logy'),
                ('nDaug', 6, 0.0, 6, 'Number of daughters of tag B', __liaison__,
                 'Number of daughters of tag B', 'Some distribution of number of daughters', 'n_{daughters}', 'Candidates'),
                ('cosThetaBetweenParticleAndNominalB', 100, -6.0, 4.0, '#cos#theta_{BY}', __liaison__,
                 'Cosine of angle between the reconstructed B and the nominal B', 'Distribution peaking between -1 and 1',
                 '#cos#theta_{BY}', 'Candidates'),
                ('d0_massDiff', 100, 0.0, 0.5, 'Mass difference of D* and D', __liaison__,
                 'Mass difference of $D^{*}$ and D', 'Peak at 0.14 GeV', 'm(D^{*})-m(D) [GeV]', 'Candidates', 'shifter'),
                ('d0_M', 100, 0.0, 3.0, 'Mass of zeroth daughter (D* or D)', __liaison__,
                 'Mass of zeroth daughter of tag B (either a $D^{*}$ or a D)', 'Peaks at 1.86 GeV and 2.00 GeV',
                 'm(D^{(*)}) [GeV]', 'Candidates', 'shifter')],
            variables_2d=[('decayModeID', 8, 0, 8, 'log10_sigProb', 100, -3.0, 0.0,
                           'Signal probability for each decay mode ID', __liaison__,
                           'Signal probability for each decay mode ID',
                           'Some distribtuion of candidates in the first few decay mode IDs',
                           'Decay mode ID', '#log_10(signal probability)', 'colz')],
            path=path)


@_FEI_skim_header("B+")
class feiSLBplus(BaseFEISkim):
    """
    Tag side :math:`B` cuts:

    * :math:`-4 < \\cos\\theta_{BY} < 3`
    * :math:`\\log_{10}(\\text{signal probability}) > -2.4`
    * :math:`p_{\\ell}^{*} > 1.0~{\\rm GeV}` in CMS frame

    SL :math:`B^+` tags are reconstructed. Hadronic :math:`B^+` with SL :math:`D` are
    not reconstructed, as these are rare and time-intensive.

    <CHANNELS>

    See also:
        `BaseFEISkim.FEIPrefix` for FEI training used, and `BaseFEISkim.fei_precuts` for
        event-level cuts made before applying the FEI.
    """
    __description__ = "FEI-tagged charged :math:`B`'s decaying semileptonically."
    validation_sample = _VALIDATION_SAMPLE

    FEIChannelArgs = {
        "neutralB": False,
        "chargedB": True,
        "hadronic": False,
        "semileptonic": True,
        "KLong": False,
        "baryonic": True,
        "removeSLD": True
    }

    def build_lists(self, path):
        ma.applyCuts("B+:semileptonic", "dmID<8", path=path)
        ma.applyCuts("B+:semileptonic", "log10_sigProb>-2.4", path=path)
        ma.applyCuts("B+:semileptonic", "-4.0<cosThetaBY<3.0", path=path)
        ma.applyCuts("B+:semileptonic", "p_lepton_CMSframe>1.0", path=path)

        self.SkimLists = ["B+:semileptonic"]

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        vm.addAlias('sigProb', 'extraInfo(SignalProbability)')
        vm.addAlias('log10_sigProb', 'log10(extraInfo(SignalProbability))')
        vm.addAlias('d0_massDiff', 'daughter(0,massDifference(0))')
        vm.addAlias('d0_M', 'daughter(0,M)')
        vm.addAlias('decayModeID', 'extraInfo(decayModeID)')
        vm.addAlias('nDaug', 'countDaughters(1>0)')  # Dummy cut so all daughters are selected.

        histogramFilename = f"{self}_Validation.root"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist='B+:semileptonic',
            variables_1d=[
                ('sigProb', 100, 0.0, 1.0, 'Signal probability', __liaison__,
                 'Signal probability of the reconstructed tag B candidates',
                 'Most around zero, with a tail at non-zero values.', 'Signal probability', 'Candidates', 'logy'),
                ('nDaug', 6, 0.0, 6, 'Number of daughters of tag B', __liaison__,
                 'Number of daughters of tag B', 'Some distribution of number of daughters', 'n_{daughters}', 'Candidates'),
                ('cosThetaBetweenParticleAndNominalB', 100, -6.0, 4.0, '#cos#theta_{BY}', __liaison__,
                 'Cosine of angle between the reconstructed B and the nominal B', 'Distribution peaking between -1 and 1',
                 '#cos#theta_{BY}', 'Candidates'),
                ('d0_massDiff', 100, 0.0, 0.5, 'Mass difference of D* and D', __liaison__,
                 'Mass difference of $D^{*}$ and D', 'Peak at 0.14 GeV', 'm(D^{*})-m(D) [GeV]', 'Candidates', 'shifter'),
                ('d0_M', 100, 0.0, 3.0, 'Mass of zeroth daughter (D* or D)', __liaison__,
                 'Mass of zeroth daughter of tag B (either a $D^{*}$ or a D)', 'Peaks at 1.86 GeV and 2.00 GeV',
                 'm(D^{(*)}) [GeV]', 'Candidates', 'shifter')],
            variables_2d=[('decayModeID', 8, 0, 8, 'log10_sigProb', 100, -3.0, 0.0,
                           'Signal probability for each decay mode ID', __liaison__,
                           'Signal probability for each decay mode ID',
                           'Some distribtuion of candidates in the first few decay mode IDs',
                           'Decay mode ID', '#log_10(signal probability)', 'colz')],
            path=path)


@_FEI_skim_header(["B0", "B+"])
class feiHadronic(BaseFEISkim):
    """
    Tag side :math:`B` cuts:

    * :math:`M_{\\text{bc}} > 5.24~{\\rm GeV}`
    * :math:`|\\Delta E| < 0.2~{\\rm GeV}`
    * :math:`\\text{signal probability} > 0.001` (omitted for decay mode 23 for
      :math:`B^+`, and decay mode 25 for :math:`B^0`)

    All available FEI :math:`B^0` and :math:`B^+` hadronic tags are reconstructed. From
    `Thomas Keck's thesis
    <https://docs.belle2.org/record/275/files/BELLE2-MTHESIS-2015-001.pdf>`_, "the
    channel :math:`B^0 \\to \\overline{D}^0 \\pi^0` was used by the FR, but is not yet
    used in the FEI due to unexpected technical restrictions in the KFitter algorithm".

    <CHANNELS>

    See also:
        `BaseFEISkim.FEIPrefix` for FEI training used, and `BaseFEISkim.fei_precuts` for
        event-level cuts made before applying the FEI.
    """
    __description__ = "FEI-tagged neutral and charged :math:`B`'s decaying hadronically."

    FEIChannelArgs = {
        "neutralB": True,
        "chargedB": True,
        "hadronic": True,
        "semileptonic": False,
        "KLong": False,
        "baryonic": True
    }

    def build_lists(self, path):
        ma.copyList("B0:feiHadronic", "B0:generic", path=path)
        ma.copyList("B+:feiHadronic", "B+:generic", path=path)
        HadronicBLists = ["B0:feiHadronic", "B+:feiHadronic"]

        for BList in HadronicBLists:
            ma.applyCuts(BList, "Mbc>5.24", path=path)
            ma.applyCuts(BList, "abs(deltaE)<0.200", path=path)

        ma.applyCuts("B+:feiHadronic", "sigProb>0.001 or extraInfo(dmID)==25", path=path)
        ma.applyCuts("B0:feiHadronic", "sigProb>0.001 or extraInfo(dmID)==23", path=path)

        self.SkimLists = HadronicBLists


@_FEI_skim_header(["B0", "B+"])
class feiSL(BaseFEISkim):
    """
    Tag side :math:`B` cuts:

    * :math:`-4 < \\cos\\theta_{BY} < 3`
    * :math:`\\log_{10}(\\text{signal probability}) > -2.4`
    * :math:`p_{\\ell}^{*} > 1.0~{\\rm GeV}` in CMS frame

    SL :math:`B^0` and :math:`B^+` tags are reconstructed. Hadronic :math:`B` with SL
    :math:`D` are not reconstructed, as these are rare and time-intensive.

    <CHANNELS>

    See also:
        `BaseFEISkim.FEIPrefix` for FEI training used, and `BaseFEISkim.fei_precuts` for
        event-level cuts made before applying the FEI.
    """
    __description__ = "FEI-tagged neutral and charged :math:`B`'s decaying semileptonically."

    FEIChannelArgs = {
        "neutralB": True,
        "chargedB": True,
        "hadronic": False,
        "semileptonic": True,
        "KLong": False,
        "baryonic": True,
        "removeSLD": True
    }

    def build_lists(self, path):
        ma.copyList("B0:feiSL", "B0:semileptonic", path=path)
        ma.copyList("B+:feiSL", "B+:semileptonic", path=path)
        SLBLists = ["B0:feiSL", "B+:feiSL"]

        Bcuts = ["log10_sigProb>-2.4", "-4.0<cosThetaBY<3.0", "p_lepton_CMSframe>1.0"]

        for BList in SLBLists:
            for cut in Bcuts:
                ma.applyCuts(BList, cut, path=path)

        self.SkimLists = SLBLists
