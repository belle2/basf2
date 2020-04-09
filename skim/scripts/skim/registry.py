# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Registry
~~~~~~~~

All skims must be registered and encoded by the relevant skim liaison.
Registering a skim is as simple as adding it to the list in ``registry.py``.

For information about the convention: https://confluence.desy.de/x/URdYBQ

"""
from basf2 import B2ERROR


class _Registry:
    """A list of all official registered skims, and their codes and modules."""
    registry = [
        # --- WG0: Systematics ---
        ("10600100", "systematics", "Systematics"),
        ("10600300", "systematics", "SystematicsTracking"),
        ("10600400", "systematics", "Resonance"),
        ("10600500", "systematics", "SystematicsRadMuMu"),
        ("10600600", "systematics", "SystematicsEELL"),
        ("10600700", "systematics", "SystematicsRadEE"),
        ("10620200", "systematics", "SystematicsLambda"),

        # --- WG1: SL + missing energy ---
        ("11110100", "semileptonic", "PRsemileptonicUntagged"),
        ("11130300", "leptonic", "LeptonicUntagged"),
        ("11160200", "semileptonic", "SLUntagged"),
        ("11180100", "fei", "feiHadronicB0"),
        ("11180200", "fei", "feiHadronicBplus"),
        ("11180300", "fei", "feiSLB0"),
        ("11180400", "fei", "feiSLBplus"),

        # --- WG2: Electroweak penguins ---
        ("12160100", "ewp", "BtoXgamma"),
        ("12160200", "ewp", "BtoXll"),
        ("12160300", "ewp", "BtoXll_LFV"),

        # --- WG3: Time-dependent CP violation ---
        ("13160100", "tcpv", "TCPV"),

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

        # --- WG5: Quarkonium ---
        ("15420100", "quarkonium", "BottomoniumEtabExclusive"),
        ("15440100", "quarkonium", "BottomoniumUpsilon"),
        ("16460100", "quarkonium", "ISRpipicc"),

        # --- WG7: Charm physics ---
        ("17230100", "charm", "XToD0_D0ToHpJm"),  # D0 -> K pi/pi pi/K K
        # D0 -> pi0 pi0/Ks pi0/Ks Ks # ("17230100", "", "D0ToHpJm"),
        # D0 -> K pi/pi pi/K K
        ("17230200", "charm", "XToD0_D0ToNeutrals"),
        ("17230300", "charm", "DstToD0Pi_D0ToRare"),  # D0 -> g g/e e/mu mu
        ("17230400", "charm", "XToDp_DpToKsHp"),  # D+ -> Ks h+
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
        ("17240700", "charm", "DstToD0Pi_D0ToHpHmKs"),  # D* -> D0 -> h h Ks
        # D* -> D0 -> K- pi+ pi0 (""+WS)
        ("17240800", "charm", "EarlyData_DstToD0Pi_D0ToHpJmPi0"),
        ("17240900", "charm", "EarlyData_DstToD0Pi_D0ToHpHmPi0"),  # D* -> D0 -> h h pi0
        # D*(tag) -> D0(tag) -> hadron, D*(q) -> D0(q) -> K+ nu l-
        ("17260900", "charm", "DstToD0Pi_D0ToSemileptonic"),

        # --- WG8: Dark matter searches and tau physics ---
        ("18020100", "dark", "SinglePhotonDark"),
        ("18020300", "dark", "ALP3Gamma"),
        ("18360100", "taupair", "TauLFV"),
        ("18520100", "dark", "DimuonPlusMissingEnergy"),
        ("18520200", "dark", "ElectronMuonPlusMissingEnergy"),
        # ("18520300", "", "DielectronPlusMissingEnergy"),
        ("18520400", "dark", "LFVZpVisible"),
        ("18570600", "taupair", "TauGeneric"),
        ("18570700", "taupair", "TauThrust"),

        # --- WG9: Charmless B decays ---
        ("14120500", "btocharmless", "BtoPi0Pi0"),  # Why is this skim code 14___ ???
        ("19130100", "btocharmless", "CharmlessHad2Body"),
        ("19130200", "btocharmless", "CharmlessHad3Body"),
    ]

    codes = [code for code, _, _ in registry]
    modules = list({module for _, module, _ in registry})
    names = [names for _, _, names in registry]

    def get_skim_module(self, SkimName):
        """Retrieve the skim module name from the registry which contains the given
        skim.

        Parameters:
            SkimName (str): Name of the skim as it appears in `skim.registry.Registry`.

        Returns:
            SkimModule (str): The name of the skim module which contains the skim.
        """
        lookup = {name: module for _, module, name in self.registry}
        try:
            return lookup[SkimName]
        except KeyError:
            B2ERROR(
                f"Unrecognised skim name {SkimName}. "
                "Please add your skim to `skim.registry.Registry`."
            )
            raise LookupError(SkimName)

    def get_expected_skims_in_module(self, SkimModule):
        """Retrieve a list of the skims listed in the registry as existing in
        the given skim module.

        Parameters:
            SkimModule (str): The name of the module, *e.g.* ``btocharmless`` (not
                ``skim.btocharmless`` or ``btocharmless.py``).

        Returns:
            ExpectedSkims (list(str)): The skims listed in the registry as belonging to
                ``SkimModule``.
        """
        if SkimModule not in self.modules:
            B2ERROR(f"Unrecognised skim module {SkimModule}.")
            raise LookupError(SkimModule)

        ModuleLookup = {name: module for _, module, name in self.registry}
        NameLookup = {
            module: [name for name in self.names if ModuleLookup[name] == module]
            for module in self.modules
        }
        return NameLookup[SkimModule]

    def encode_skim_name(self, SkimName):
        """Find the 8 digit skim code assigned to the skim with the provided name.

        Parameters:
            SkimName (str): Name of the skim as it appears in `skim.registry.Registry`.

        Returns:
            SkimCode (str): 8 digit skim code assigned to the given skim.
        """
        lookup = {name: code for code, _, name in self.registry}
        try:
            return lookup[SkimName]
        except KeyError:
            B2ERROR(
                f"Unrecognised skim name {SkimName}. "
                "Please add your skim to `skim.registry.Registry`."
            )
            raise LookupError(SkimName)

    def decode_skim_code(self, SkimCode):
        """Find the name of the skim which corresponds to the provided skim code.

        This is useful to determine the skim script used to produce a specific uDST
        file, given the 8-digit code name of the file itself.

        Parameters:
            SkimCode (str): 8 digit skim code assigned to some skim.

        Returns:
            SkimName (str): Name of the corresponding skim as it appears in
            `skim.registry.Registry`.
        """
        lookup = {code: name for code, _, name in self.registry}
        try:
            return lookup[SkimCode]
        except KeyError:
            B2ERROR(
                f"Unrecognised skim code {SkimCode}. "
                "Please add your skim to `skim.registry.Registry`."
            )
            raise LookupError(SkimCode)


# Make a publicly accessible Registry instance
Registry = _Registry()

#: A dict listing which standalone skims are included the combined skim scripts
# DELETE: Will be made obselete by CombinedSkim class & autogeneration of combined skims
combined_skims = {
    "BtoCharm1": [
        "BtoD0h_hh", "BtoD0h_Kshh", "BtoD0h_Kspi0", "BtoD0h_Kspipipi0", "B0toDpi_Kspi",
        "B0toDrho_Kspi", "B0toDpi_Kpipi", "B0toDrho_Kpipi",
    ],
    "BtoCharm2": [
        "BtoD0h_Kpipipi_Kpipi0", "B0toDstarPi_D0pi_Kpipipi_Kpipi0",
        "B0toDstarRho_D0pi_Kpipipi_Kpipi0", "BtoD0rho_Kpipipi_Kpipi0", "BtoD0h_Kpi",
        "B0toDstarPi_D0pi_Kpi", "BtoD0rho_Kpi", "B0toDstarRho_D0pi_Kpi",
    ],
    'CharmHigh': [
        'DstToD0Pi_D0ToHpJmPi0', 'XToD0_D0ToNeutrals', 'DstToD0Pi_D0ToHpHmPi0'
    ],
    'CharmLow': [
        'XToD0_D0ToHpJm', 'DstToD0Pi_D0ToHpJm', 'DstToD0Pi_D0ToHpHmKs',
        'DstToD0Pi_D0ToRare'
    ],
    'CharmEarlyData': [
        'EarlyData_DstToD0Pi_D0ToHpJmPi0', 'EarlyData_DstToD0Pi_D0ToHpHmPi0'
    ],
    'Dark': [
        'ALP3Gamma', 'SinglePhotonDark', 'LFVZpVisible', 'DimuonPlusMissingEnergy',
        'ElectronMuonPlusMissingEnergy'  # 'DielectronPlusMissingEnergy'
    ],
    'EWP': [
        'BtoXll', 'BtoXll_LFV', 'BtoXgamma'
    ],
    'feiHadronicCombined': [
        'feiHadronicB0', 'feiHadronicBplus'
    ],
    'feiSLCombined': [
        'feiSLB0', 'feiSLBplus'
    ],
    'fei': [
        'feiHadronicB0', 'feiHadronicBplus', 'feiSLB0', 'feiSLBplus'
    ],
    'TCPV': [
        'TCPV',
    ],
    'Leptonic': [
        'LeptonicUntagged'
    ],
    'TauCombined': [
        'TauLFV', 'TauGeneric', 'TauThrust'
    ]
}
