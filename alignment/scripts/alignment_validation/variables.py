##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
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
        """Initialize a unit definition.

        Parameters
        ----------
        name : str
            Unit label used on standard axes (e.g. ``" [cm]"``).
        dname : str
            Unit label used on residual/difference axes.
        multiplier : float
            Conversion factor from ``name`` units to ``dname`` units.
        """
        #: Unit name shown in plots.
        self.name = name
        #: Unit name used for residual axes.
        self.dname = dname
        #: Conversion factor from ``name`` to ``dname`` units.
        self.convert = multiplier


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
        """Initialize a global variable description.

        Parameters
        ----------
        name : str
            ROOT branch name.
        latex : str
            LaTeX string for axis labels.
        unit : Unit
            Unit instance describing display and residual units.
        plaintext : str
            Short plain-text identifier used for file names.
        """
        #: ROOT branch name.
        self.name = name
        #: LaTeX string for axis labels.
        self.latex = latex
        #: Unit descriptor for plotting and residuals.
        self.unit = unit
        #: Plain-text identifier for file naming.
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
        """Initialize a two-track variable description.

        Parameters
        ----------
        name1 : str
            ROOT branch name for the first track.
        name2 : str
            ROOT branch name for the second track.
        latex : str
            LaTeX string for axis labels.
        unit : Unit
            Unit instance describing display and residual units.
        plaintext : str
            Short plain-text identifier used for file names.
        """
        #: ROOT branch name for the first track.
        self.name1 = name1
        #: ROOT branch name for the second track.
        self.name2 = name2
        #: LaTeX string for axis labels.
        self.latex = latex
        #: Unit descriptor for plotting and residuals.
        self.unit = unit
        #: Plain-text identifier for file naming.
        self.plaintext = plaintext

    def getName(self):
        """Return a list of both ROOT branch names (track 1 then track 2).

        Returns
        -------
        list of str
        """
        return [self.name1, self.name2]


# Unit instances
#: Seconds unit (no conversion).
s = Unit(" [s]", "s", 1)
#: Centimeters with residuals in micrometers.
cm = Unit(" [cm]", r" [$\mu$m]", 1e4)
#: Radians with residuals in milliradians.
rad = Unit(" [rad]", " [mrad]", 1e3)
#: Dimensionless with residuals in 1e-3.
unit = Unit(" [1]", r" [$10^{-3}$]", 1e3)
#: Inverse centimeters with scaled residuals.
inverse_cm = Unit(" [1/cm]", r" [1/cm $\cdot 10^{-4}$]", 1e4)
#: GeV/c (no conversion).
gev = Unit(" [GeV/c]", r" [GeV/c]", 1)
