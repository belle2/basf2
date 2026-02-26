"""
Physics variable metadata classes and unit definitions for alignment validation.
"""


class Unit:
    """Pair of display/residual unit strings and the conversion factor between them.

    Parameters
    ----------
    name : str
        Unit label used on standard axes (e.g. ``" [cm]"``).
    dname : str
        Unit label used on residual/difference axes, after applying the
        conversion factor (e.g. ``r" [$\\mu$m]"``).
    multiplier : float
        Factor to convert from ``name`` units to ``dname`` units
        (e.g. ``1e4`` to go from cm to μm).
    """

    def __init__(self, name, dname, multiplier):
        self.name = name        # Unit name shown in plots
        self.dname = dname      # Unit name used for residual axes
        self.convert = multiplier  # Conversion factor from name to dname


class GlobalVariable:
    """Metadata for a scalar observable stored in a single ROOT branch per event.

    Parameters
    ----------
    name : str
        ROOT branch name.
    latex : str
        LaTeX string for axis labels (e.g. ``r"$M_{inv}$"``).
    unit : Unit
        Unit instance describing display and residual units.
    plaintext : str
        Short plain-text identifier used for file names (e.g. ``"run"``).
    """

    def __init__(self, name, latex, unit, plaintext):
        self.name = name
        self.latex = latex
        self.unit = unit
        self.plaintext = plaintext

    def getName(self):
        """Return a list containing the single ROOT branch name.

        Returns
        -------
        list of str
        """
        return [self.name]


class TrackVariable:
    """Metadata for an observable with one ROOT branch per track.

    Used for two-track events (cosmics, dimuons) where each track has its
    own branch.

    Parameters
    ----------
    name1 : str
        ROOT branch name for the first track.
    name2 : str
        ROOT branch name for the second track.
    latex : str
        LaTeX string for axis labels (e.g. ``r"d$_0$"``).
    unit : Unit
        Unit instance describing display and residual units.
    plaintext : str
        Short plain-text identifier used for file names (e.g. ``"d"``).
    """

    def __init__(self, name1, name2, latex, unit, plaintext):
        self.name1 = name1
        self.name2 = name2
        self.latex = latex
        self.unit = unit
        self.plaintext = plaintext

    def getName(self):
        """Return a list of both ROOT branch names (track 1 then track 2).

        Returns
        -------
        list of str
        """
        return [self.name1, self.name2]


# Unit instances
s = Unit(" [s]", "s", 1)
cm = Unit(" [cm]", r" [$\mu$m]", 1e4)
rad = Unit(" [rad]", " [mrad]", 1e3)
unit = Unit(" [1]", r" [$10^{-3}$]", 1e3)
inverse_cm = Unit(" [1/cm]", r" [1/cm $\cdot 10^{-4}$]", 1e4)
gev = Unit(" [GeV/c]", r" [GeV/c]", 1)
