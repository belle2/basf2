# !/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from importlib import import_module
import pandas as pd

from basf2 import B2ERROR

from tabulate import tabulate


_RegisteredSkims = [
    # --- WG0: Systematics ---
    ("10000000", "systematics", "Random"),
    # ("10600100", "systematics", "Systematics"), renamed to SystematicsDstar.
    ("10600300", "systematics", "SystematicsTracking"),
    ("10600400", "systematics", "Resonance"),
    ("10600500", "systematics", "SystematicsRadMuMu"),
    ("10600600", "systematics", "SystematicsEELL"),
    ("10600700", "systematics", "SystematicsRadEE"),
    ("10620200", "systematics", "SystematicsLambda"),
    ("11640100", "systematics", "SystematicsPhiGamma"),
    ("10600800", "systematics", "SystematicsFourLeptonFromHLTFlag"),
    ("10600900", "systematics", "SystematicsRadMuMuFromHLTFlag"),
    ("10611000", "systematics", "SystematicsJpsi"),
    ("10611100", "systematics", "SystematicsKshort"),
    ("10601200", "systematics", "SystematicsBhabha"),
    ("10601300", "systematics", "SystematicsCombinedHadronic"),
    ("10601400", "systematics", "SystematicsCombinedLowMulti"),
    ("10601500", "systematics", "SystematicsDstar"),

    # --- WG1: SL + missing energy ---
    ("11110100", "semileptonic", "PRsemileptonicUntagged"),
    ("11130300", "leptonic", "LeptonicUntagged"),
    ("11130301", "leptonic", "dilepton"),
    ("11160200", "semileptonic", "SLUntagged"),
    ("11160201", "semileptonic", "B0toDstarl_Kpi_Kpipi0_Kpipipi"),
    ("11180100", "fei", "feiHadronicB0"),
    ("11180200", "fei", "feiHadronicBplus"),
    ("11180300", "fei", "feiSLB0"),
    ("11180400", "fei", "feiSLBplus"),
    ("11180500", "fei", "feiHadronic"),
    ("11180600", "fei", "feiSL"),

    # --- WG2: Electroweak penguins ---
    ("12160100", "ewp", "BtoXgamma"),
    ("12160200", "ewp", "BtoXll"),
    ("12160300", "ewp", "BtoXll_LFV"),
    ("12160400", "ewp", "inclusiveBplusToKplusNuNu"),

    # --- WG3: Time-dependent CP violation ---
    ("13160200", "tdcpv", "TDCPV_ccs"),
    ("13160300", "tdcpv", "TDCPV_qqs"),

    # --- WG4: Charmed B decays ---
    ("14120300", "btocharm", "BtoD0h_Kspi0"),
    ("14120400", "btocharm", "BtoD0h_Kspipipi0"),
    # B0 -> D-(k+ ""- pi-)pi+ # ("14140500", "", "BtoD0h_Kspi0pi0"),
    # Add when skim script is ready
    ("14120600", "btocharm", "B0toDpi_Kpipi"),
    ("14120601", "btocharm", "B0toDpi_Kspi"),   # B0 -> D-(Ks pi-)pi+
    # B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> k+ pi-
    ("14120700", "btocharm", "B0toDstarPi_D0pi_Kpi"),
    # merge  B0 -> D*-(anti-D0 pi-)pi+ with anti-D0 -> k- pi+ pi+ pi-
    # and anti-D0 -> K- pi+ pi0
    ("14120800", "btocharm", "B0toDstarPi_D0pi_Kpipipi_Kpipi0"),
    ("14121100", "btocharm", "B0toDrho_Kpipi"),
    ("14121101", "btocharm", "B0toDrho_Kspi"),
    ("14121200", "btocharm", "B0toDstarRho_D0pi_Kpi"),
    ("14121201", "btocharm", "B0toDstarRho_D0pi_Kpipipi_Kpipi0"),
    ("14140100", "btocharm", "BtoD0h_hh"),
    ("14140101", "btocharm", "BtoD0h_Kpi"),
    # B+ -> anti-D0/anti-D0* (K- pi+ pi+ pi-, K- ""+ pi0) h+
    ("14140102", "btocharm", "BtoD0h_Kpipipi_Kpipi0"),
    ("14140200", "btocharm", "BtoD0h_Kshh"),
    ("14141000", "btocharm", "BtoD0rho_Kpi"),
    ("14141001", "btocharm", "BtoD0rho_Kpipipi_Kpipi0"),
    ("14141002", "btocharm", "B0toDD_Kpipi_Kspi"),
    ("14141003", "btocharm", "B0toDstarD"),
    ("14140104", "btocharm", "B0toD0Kpipi0_pi0"),


    # --- WG5: Quarkonium ---
    ("15410300", "quarkonium", "InclusiveLambda"),
    ("15420100", "quarkonium", "BottomoniumEtabExclusive"),
    ("15440100", "quarkonium", "BottomoniumUpsilon"),
    # ("16460100", "quarkonium", "ISRpipicc"), Subset of 16460200, deleted.
    ("16460200", "quarkonium", "CharmoniumPsi"),

    # --- WG7: Charm physics ---
    ("17230100", "charm", "XToD0_D0ToHpJm"),  # D0 -> K pi/pi pi/K K
    # D0 -> pi0 pi0/Ks pi0/Ks Ks # ("17230100", "", "D0ToHpJm"),
    # D0 -> K pi/pi pi/K K
    ("17230200", "charm", "XToD0_D0ToNeutrals"),
    ("17230300", "charm", "DstToD0Pi_D0ToRare"),  # D0 -> g g/e e/mu mu
    ("17230400", "charm", "XToDp_DpToKsHp"),  # D+ -> Ks h+
    ("17230500", "charm", "XToDp_DpToHpHmJp"),  # D+ -> h+ h- j+
    ("17230600", "charm", "LambdacTopHpJm"),  # Lambda_c+ -> proton h- j+
    ("17240100", "charm", "DstToD0Pi_D0ToHpJm"),  # D* -> D0 -> K pi/pi pi/K K
    # D* -> D0 -> K- pi+ pi0 (""+WS)
    ("17240200", "charm", "DstToD0Pi_D0ToHpJmPi0"),
    ("17240300", "charm", "DstToD0Pi_D0ToHpHmPi0"),  # D* -> D0 -> h h pi0
    # D* -> D0 -> Ks omega / Ks eta -> Ks pi+ pi- pi0
    ("17240400", "charm", "DstToD0Pi_D0ToKsOmega"),
    # D* -> D0 -> K- pi+ eta (""+WS)
    ("17240500", "charm", "DstToD0Pi_D0ToHpJmEta"),
    # D* -> D0 -> pi0 pi0/Ks pi0/Ks Ks
    ("17240600", "charm", "DstToD0Pi_D0ToNeutrals"),
    ("17240700", "charm", "DstToD0Pi_D0ToHpJmKs"),  # D* -> D0 -> h h Ks
    # D* -> D0 -> K- pi+ pi0 (""+WS)
    ("17240800", "charm", "EarlyData_DstToD0Pi_D0ToHpJmPi0"),
    ("17240900", "charm", "EarlyData_DstToD0Pi_D0ToHpHmPi0"),  # D* -> D0 -> h h pi0
    ("17241000", "charm", "DstToDpPi0_DpToHpPi0"),  # D*+ -> D+ pi0, D+ -> h+ pi0
    ("17241100", "charm", "DstToD0Pi_D0ToHpHmHpJm"),  # D* -> D0 -> h h h j

    # --- WG8: Dark matter searches and tau physics ---
    ("18020100", "dark", "SinglePhotonDark"),
    ("18020200", "dark", "GammaGammaControlKLMDark"),
    ("18020300", "dark", "ALP3Gamma"),
    ("18020400", "dark", "EGammaControlDark"),
    ("18000000", "dark", "InelasticDarkMatter"),
    ("18000001", "dark", "RadBhabhaV0Control"),
    ("18360100", "taupair", "TauLFV"),
    ("18520100", "dark", "DimuonPlusMissingEnergy"),
    ("18520200", "dark", "ElectronMuonPlusMissingEnergy"),
    ("18520300", "dark", "DielectronPlusMissingEnergy"),
    ("18520400", "dark", "LFVZpVisible"),
    ("18570600", "taupair", "TauGeneric"),
    ("18570700", "taupair", "TauThrust"),
    ("18530100", "lowMulti", "TwoTrackLeptonsForLuminosity"),
    ("18520500", "lowMulti", "LowMassTwoTrack"),
    ("18530200", "lowMulti", "SingleTagPseudoScalar"),

    # --- WG9: Charmless B decays ---
    ("19120100", "btocharmless", "BtoPi0Pi0"),
    ("19130201", "btocharmless", "BtoHadTracks"),
    ("19130300", "btocharmless", "BtoHad1Pi0"),
    ("19130310", "btocharmless", "BtoHad3Tracks1Pi0"),
]
"""
A list of all official registered skims and their skim code and parent module. Entries
must be of the form ``(code, module, name)``.
"""


def _add_skim_registry_table(SkimRegistry):
    """
    Decorator to add a Sphinx table to the docstring of the skim registry.

    Inserts table wherever '<TABLE>' is in the docstring.
    """

    df = pd.DataFrame(_RegisteredSkims, columns=["Skim code", "Module", "Skim name"])
    df = df[["Module", "Skim name", "Skim code"]].sort_values(by=["Module", "Skim code"])
    table = tabulate(df, showindex="never", tablefmt="grid", headers=df.columns)

    # Manual text manipulation (read: filthy hack) to make the table hierarchical
    OriginalLines = table.split("\n")
    header, OriginalLines, footer = OriginalLines[:2], OriginalLines[2:-1], OriginalLines[-1]
    CurrentModule = ""
    lines = []
    lines.append("\n    ".join(header))
    for BorderLine, TextLine in zip(OriginalLines[::2], OriginalLines[1::2]):
        segments = TextLine.split("|")
        module = segments[1].lstrip().rstrip()
        if CurrentModule == module:
            segments[1] = " " * len(segments[1])
            BorderLine = "|" + " " * len(segments[1]) + BorderLine.lstrip("+").lstrip("-")
        else:
            CurrentModule = module
        lines.append(BorderLine)
        lines.append("|".join(segments))
    lines.append(footer)

    SkimRegistry.__doc__ = SkimRegistry.__doc__.replace("<TABLE>", "\n    ".join(lines))

    return SkimRegistry


@_add_skim_registry_table
class SkimRegistryClass:
    """
    Class containing information on all official registered skims. This class also
    contains helper functions for getting information from the registry. For
    convenience, an instance of this class is provided: `skim.registry.Registry`.

    The table below lists all registered skims and their skim codes:

    <TABLE>
    """
    _registry = _RegisteredSkims

    def __init__(self):
        self._codes = [code for code, _, _ in self._registry]
        self._modules = list({module for _, module, _ in self._registry})
        self._names = [names for _, _, names in self._registry]

    @property
    def names(self):
        """A list of all registered skim names."""
        return self._names

    @property
    def codes(self):
        """A list of all registered skim codes."""
        return self._codes

    @property
    def modules(self):
        """A list of all registered skim modules."""
        return self._modules

    def get_skim_module(self, SkimName):
        """Retrieve the skim module name from the registry which contains the given
        skim.

        Parameters:
            SkimName (str): Name of the skim as it appears in the skim registry.

        Returns:
            The name of the skim module which contains the skim.
        """
        lookup = {name: module for _, module, name in self._registry}
        try:
            return lookup[SkimName]
        except KeyError:
            B2ERROR(
                f"Unrecognised skim name {SkimName}. "
                "Please add your skim to the list in `skim/scripts/skim/registry.py`."
            )
            raise LookupError(SkimName)

    def get_skims_in_module(self, SkimModule):
        """Retrieve a list of the skims listed in the registry as existing in
        the given skim module.

        Parameters:
            SkimModule (str): The name of the module, *e.g.* ``btocharmless`` (not
                ``skim.btocharmless`` or ``btocharmless.py``).

        Returns:
            The skims listed in the registry as belonging to ``SkimModule``.
        """
        if SkimModule not in self.modules:
            B2ERROR(f"Unrecognised skim module {SkimModule}.")
            raise LookupError(SkimModule)

        ModuleLookup = {name: module for _, module, name in self._registry}
        NameLookup = {
            module: [name for name in self.names if ModuleLookup[name] == module]
            for module in self.modules
        }
        return NameLookup[SkimModule]

    def get_skim_function(self, SkimName):
        """Get the skim class constructor for the given skim.

        This is achieved by importing the module listed alongside the skim name in the
        skim registry.

        Parameters:
            SkimName (str): Name of the skim to be found.

        Returns:
            The class constructor for the given skim.
        """
        ModuleName = self.get_skim_module(SkimName)
        SkimModule = import_module(f"skim.WGs.{ModuleName}")
        return getattr(SkimModule, SkimName)

    def encode_skim_name(self, SkimName):
        """Find the 8 digit skim code assigned to the skim with the provided name.

        Parameters:
            SkimName (str): Name of the corresponding skim as it appears in the skim registry.

        Returns:
            8 digit skim code assigned to the given skim.
        """
        lookup = {name: code for code, _, name in self._registry}
        try:
            return lookup[SkimName]
        except KeyError:
            B2ERROR(
                f"Unrecognised skim name {SkimName}. "
                "Please add your skim to the list in `skim/scripts/skim/registry.py`."
            )
            raise LookupError(SkimName)

    def decode_skim_code(self, SkimCode):
        """Find the name of the skim which corresponds to the provided skim code.

        This is useful to determine the skim script used to produce a specific uDST
        file, given the 8-digit code name of the file itself.

        Parameters:
            SkimCode (str): 8 digit skim code assigned to some skim.

        Returns:
            Name of the corresponding skim as it appears in the skim registry.
        """
        lookup = {code: name for code, _, name in self._registry}
        try:
            return lookup[SkimCode]
        except KeyError:
            B2ERROR(
                f"Unrecognised skim code {SkimCode}. "
                "Please add your skim to the list in `skim/scripts/skim/registry.py`."
            )
            raise LookupError(SkimCode)


Registry = SkimRegistryClass()
"""
An instance of `skim.registry.SkimRegistryClass`. Use this in your script to get
information from the registry.

    >>> from skim.registry import Registry
    >>> Registry.encode_skim_name("SinglePhotonDark")
    18020100
"""
