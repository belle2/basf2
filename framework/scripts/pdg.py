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
