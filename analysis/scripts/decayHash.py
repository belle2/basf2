#!/usr/bin/env python3
# Thomas Keck

import sys
import re
import struct
import pdg
import basf2

from pybasf2 import *
# inspect is also used by LogPythonInterface. Do not remove
import inspect
logging = LogPythonInterface()

import numpy as np
import collections

import ROOT
ROOT.gSystem.Load("libanalysis.so")
ROOT.gSystem.Load("libanalysis_utility.so")
from ROOT import Belle2


def _bitwiseConversion(value, i='f', o='i'):
    """
    Bitwise conversion between to python types
    This is equivalently to
    union {
        Type_i i;
        Type_o o;
    } conversion;
    conversion.i = input;
    return conversion.o
    @param i input data type (e.g. f for float)
    @param o output data type (e.g. i for integer)
    """
    s = struct.pack('>' + i, value)
    return struct.unpack('>' + o, s)[0]


def _decayHashFloatToInt(decayHash, decayHashExtended):
    """
    Convert decayHash and decayHashExtended 32 bit floats to an 64 bit integer
    """
    decayHashInt = _bitwiseConversion(np.float32(decayHash))
    decayHashExtendedInt = _bitwiseConversion(np.float32(decayHashExtended))
    decayHashFullInt = decayHashInt << 32
    decayHashFullInt += decayHashExtendedInt
    return decayHashFullInt


class DecayHashMap(object):
    """
    DecayHashMap using the C++ implementation of DecayTree and DecayNode
    """
    def __init__(self, rootfile, removeRadiativeGammaFlag=False):
        """Constructor"""
        import root_numpy
        ntuple = root_numpy.root2array(rootfile)
        # self._removeGammaFlag = removeRadiativeGammaFlag
        #: Dict Int -> DecayStrings
        self._string = {}
        #: Dict Int -> Reconstructed DecayTree
        self._forest = {}
        for decayHash, decayHashExtended, decayString in ntuple:
            decayInt = Belle2.DecayForest.decayHashFloatToInt(decayHash, decayHashExtended)
            if decayInt in self._string:
                continue
            self._string[decayInt] = decayString
            self._forest[decayInt] = Belle2.DecayForest(decayString, True, removeRadiativeGammaFlag)

    def get_string(self, decayHash, decayHashExtended):
        """
        Return DecayString given the decayHash and decayHashExtended
        @param decayHash output of extraInfo(decayHash)
        @param decayHashExtended output of extraInfo(decayHashExtended)
        """
        return self._string[Belle2.DecayForest.decayHashFloatToInt(decayHash, decayHashExtended)]

    def get_original_decay(self, decayHash, decayHashExtended):
        """
        Return original (MC) DecayTree given the decayHash and decayHashExtended
        @param decayHash output of extraInfo(decayHash)
        @param decayHashExtended output of extraInfo(decayHashExtended)
        """
        return self._forest[Belle2.DecayForest.decayHashFloatToInt(decayHash, decayHashExtended)].getOriginalTree()

    def get_reconstructed_decay(self, decayHash, decayHashExtended):
        """
        Return reconstructed DecayTree given the decayHash and decayHashExtended
        @param decayHash output of extraInfo(decayHash)
        @param decayHashExtended output of extraInfo(decayHashExtended)
        """
        return self._forest[Belle2.DecayForest.decayHashFloatToInt(decayHash, decayHashExtended)].getReconstructedTree()

    def print_hash(self, decayHash, decayHashExtended):
        """
        Print the DecayString in a fancy way given the decayHash and decayHashExtended
        @param decayHash output of extraInfo(decayHash)
        @param decayHashExtended output of extraInfo(decayHashExtended)
        """
        entry = self.get_string(decayHash, decayHashExtended)
        entries = entry.split('|')
        all_particles = re.findall(r"(-?[0-9]+)", entries[0])

        if len(all_particles) != len(entries) - 1:
            print(entry)
            raise RuntimeError("Bad format of decay string: " +
                               str(len(all_particles)) + " " + str(len(entries)) + " " + str(entries))

        table = []
        table.append(["Decay ", prettify_pdg_codes(entries[0])])
        for particle, mc_decay_string in zip(all_particles, entries[1:]):
            table.append([prettify_pdg_codes(particle), prettify_pdg_codes(mc_decay_string)])

        basf2.pretty_print_table(table, column_widths=[6, '*'])


def _pdg_to_name(x):
    """
    Convert PDG code to a name
    @param a pdg code
    """
    selected = False
    if x[0] == '^':
        selected = True
        x = x[1:]

    pdg_code = int(x)
    pdg_string = str(pdg_code)
    try:
        pdg_string = pdg.to_name(pdg_code)
    except:
        pass

    if selected:
        if LogPythonInterface.terminal_supports_colors():
            return '\x1b[31m' + pdg_string + '\x1b[0m'
        else:
            return '^' + pdg_string
    return pdg_string


def prettify_pdg_codes(text):
    """
    Prettifiy a string containing PDG codes by replacing PDG codes
    with their corresponding names.
    @param text the text
    """
    text = re.sub(r"(\^?-?[0-9]+)", lambda x: _pdg_to_name(x.group(0)), text)
    text = text.replace('gamma', 'g').replace('--> ', '').replace('anti-', 'a-')
    text = text.replace('Upsilon', 'Y').replace(') ', ')').replace(' (', '(')
    return text
