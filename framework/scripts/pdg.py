#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
pdg - access particle definitions
---------------------------------

This module helps to access particle definitions. When the software is loaded a
list of known particles is read from the EvtGen particle definition file
:file:`{$BELLE2_EXTERNALS_DIR}/share/evtgen/evt.pdl`. This file contains all
well known standard model particles and their properties: mass, width, charge,
...

This module allows to easily access this information (see `get`) or if necessary
add new particles using `add_particle` and even replace the whole particle
definition list using `load`.

It also provides simple getters to convert `PDG codes`_ into particle names and
vice versa for use with modules which require a list of PDG codes for the
particles to generate. See `from_name`, `from_names`, `to_name` and `to_names`

.. _PDG codes: http://pdg.lbl.gov/2017/reviews/rpp2017-rev-monte-carlo-numbering.pdf
"""

import re
import basf2
import ROOT
from ROOT.Belle2 import EvtGenDatabasePDG

# the particle database (filled from evt.pdl by framework)
_database = EvtGenDatabasePDG.Instance()


def get(name):
    """
    Function to return particle information (TParticlePDG) from ROOT Database.

    'name' can be either the name of the particle, or a pdg code.
    Will throw an LookupError of no such particle exists.
    """

    p = _database.GetParticle(name)
    if not p:
        raise LookupError("No particle with name '%s'" % name)

    return p


def from_name(name):
    """
    Function to return pdg code for the given particle name.

    >>> pdg.from_name("pi+")
    211
    """

    return get(name).PdgCode()


def from_names(names):
    """
    for a list/tuple of particle names, return list of pdg codes.

    >>> pdg.from_names(["e+","e-","gamma"])
    [-11, 11, 22]
    """

    assert not isinstance(names, str), 'Argument is not a list!'

    return [from_name(n) for n in names]


def to_name(pdg):
    """
    Return particle name for given pdg code.

    >>> pdg.to_name(321)
    K+
    """

    return get(pdg).GetName()


def to_names(pdg_codes):
    """
    for a list/tuple of pdg codes, return list of paricle names.

    >>> pdg.to_names([11, -11, -211, 3212])
    ['e-', 'e+', 'pi-', 'Sigma0']
    """

    assert not isinstance(pdg_codes, int), 'Argument is not a list!'

    return [to_name(pdg) for pdg in pdg_codes]


def conjugate(name):
    """
    Function to return name of conjugated particle
    """

    try:
        return to_name(-from_name(name))
    except LookupError as e:
        return name


def load(filename):
    """
    Read particle database from given evtgen pdl file
    """
    _database.ReadEvtGenTable(filename)


def load_default():
    """Read default evt.pdl file"""
    _database.ReadEvtGenTable()


def add_particle(name, pdgCode, mass, width, charge, spin, max_width=None, lifetime=0, pythiaID=0):
    """
    Add a new particle to the list of known particles.

    The name cannot contain any whitespace character.

    Args:
        name (str): name of the particle
        pdgCode (int): pdg code identifiert for the particle
        mass (float): mass of the particle in GeV
        width (float): width of the particle in GeV
        charge (float): charge of the particle in e
        sping (float): spin of the particle
        max_width (float): max width, if omitted 3*width will be used
        lifetime (float): lifetime in ns, should be 0 as geant4 cannot handle it correctly otherwise
        pythiaID (int): pythiaID of the particle (if any), if omitted 0 will be used
    """
    if lifetime > 0:
        basf2.B2WARNING("Userdefined particle with non-zero lifetime will not be simulated correctly")

    if max_width is None:
        # FIXME: is 3 a good default?
        max_width = width * 3

    particle = _database.AddParticle(name, name, mass, False, width, charge * 3, "userdefined",
                                     pdgCode, 0, 0, lifetime, spin, max_width, pythiaID)
    if particle:
        basf2.B2INFO("Adding new particle '%s' (pdg=%d, mass=%.3g GeV, width=%.3g GeV, charge=%d, spin=%d)" %
                     (name, pdgCode, mass, width, charge, spin))
        return True

    return False


def search(name=None, min_mass=None, max_mass=None, name_regex=False, include_width=False):
    """
    Search for a particles by name or mass or both.

    This function allows to search for particle by name or mass and will return
    a list of all particles which match the given criteria.

    By default all searches for the name are case insensitive but if ``name``
    starts with "~" the search will be case sensitive. The "~" will not be part
    of the search.

    If ``name_regex=True`` the name will be interpreted as a python
    :py:mod:`regular expression <re>` and the function will return all particles
    whose names match the expression.  If ``name_regex=False`` the function will
    return a list of all particles containing the given pattern as substring
    ignoring case with two special cases:

    - if ``name`` begins with "^", only particles
      beginning with the pattern will be searched. The "^" will not be part of the search.
    - if ``name`` ends with "$" the pattern will only be matched to the end
      of the particle name. The "$" will not be part of the search.

    If ``include_width=True`` the search will include all particles if their
    (mass ± width) is within the given limit. If ``include_width`` is a positive
    number then the particle will be returned if $m ± n*\Gamma$ is within the
    required range where n is the value of ``include_width``

    Examples:
        Search for all particles containing a "pi" somewhere in the name and ignore the case

        >>> search_name("pi")

        Search for all particles beginning with K or k

        >>> search_name("^K")

        Search for all particles ending with "+" and having a maximal mass of 3 GeV:

        >>> search_name("+$", max_mass=3.0)

        Search for all particles which contain a capital D and have a minimal mass of 1 GeV

        >>> search_name("~D", min_mass=1.0)

        Search for all partiles which contain a set of parenthesis containing a number

        >>> search_name(".*\(\d*\).*", regex=True)

        Search all particles whose mass ± width covers 1 to 1.2 GeV

        >>> search_mass(min_mass=1.0, max_mass=1.2, include_width=True)

        Search all particles whose mass ± 3*width touches 1 GeV

        >>> search_mass(min_mass=1.0, max_mass=1.0, include_width=3)


    Parameters:
        name (str): Search pattern which will either be matched as a substring
            or as regular expression if ``name_regex=True``
        min_mass (float): minimal mass for all returned particles or None for no limit
        max_mass (float): maximal mass for all returned particles or None for no limit
        name_regex (bool): if True then ``name`` will be treated as a regula expression
        include_width (float or bool): if True or >0 include the particles if
            (mass ± include_width*width) falls within the mass limits """

    pattern = None
    if name is not None:
        options = re.IGNORECASE
        if name[0] == "~":
            name = name[1:]
            options = 0

        if name_regex is False:
            if name[0] == "^":
                name = "^{}.*".format(re.escape(name[1:]))
            elif name[-1] == "$":
                name = ".*{}$".format(re.escape(name[:-1]))
            else:
                name = ".*{}.*".format(re.escape(name))

        pattern = re.compile(name, options)

    if include_width is True:
        include_width = 1

    if include_width < 0:
        include_width = 0

    result = []
    for p in _database.ParticleList():
        if pattern is not None and not pattern.match(p.GetName()):
            continue
        m = p.Mass()
        w = p.Width() * include_width
        if min_mass is not None and min_mass > (m+w):
            continue
        if max_mass is not None and max_mass < (m-w):
            continue
        result.append(p)

    return result
