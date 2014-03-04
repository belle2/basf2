#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Small wrapper module to ease access to pdg codes for particle gun or similar
usage in the steering file
"""

from ROOT import TDatabasePDG

## the particle database (filled from evt.pdl by framework)
database = TDatabasePDG.Instance()


def get(name):
    """
    Function to return particle information (TParticlePDG) from ROOT Database.

    'name' can be either the name of the particle, or a pdg code.
    Will throw an LookupError of no such particle exists.
    """

    p = database.GetParticle(name)
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

    >>> pdg.to_names([11,-11,-211,3212])
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
    except LookupError, e:
        return name


