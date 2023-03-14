#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import re
import struct
import pdg
import basf2
import pybasf2
import uproot

import numpy as np


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


class DecayHashMap:
    """
    DecayHashMap using the C++ implementation of DecayTree and DecayNode
    """

    def __init__(self, rootfile, removeRadiativeGammaFlag=False):
        """Constructor"""
        # Always avoid the top-level 'import ROOT'.
        import ROOT  # noqa
        with uproot.open(rootfile) as rf:
            trees = rf.keys()
            assert len(trees) == 1
            ntuple = rf[trees[0]].arrays(library='np')
        # self._removeGammaFlag = removeRadiativeGammaFlag
        #: Dict Int -> DecayStrings
        self._string = {}
        #: Dict Int -> Reconstructed DecayTree
        self._forest = {}
        for decayHash, decayHashExtended, decayString in zip(
                ntuple['decayHash'], ntuple['decayHashExtended'], ntuple['decayString']):
            decayInt = ROOT.Belle2.DecayForest.decayHashFloatToInt(decayHash, decayHashExtended)
            if decayInt in self._string:
                continue
            self._string[decayInt] = decayString
            self._forest[decayInt] = ROOT.Belle2.DecayForest(decayString, True, removeRadiativeGammaFlag)

    def get_string(self, decayHash, decayHashExtended):
        """
        Return DecayString given the decayHash and decayHashExtended
        @param decayHash output of extraInfo(decayHash)
        @param decayHashExtended output of extraInfo(decayHashExtended)
        """
        # Always avoid the top-level 'import ROOT'.
        import ROOT  # noqa
        return self._string[ROOT.Belle2.DecayForest.decayHashFloatToInt(decayHash, decayHashExtended)]

    def get_original_decay(self, decayHash, decayHashExtended):
        """
        Return original (MC) DecayTree given the decayHash and decayHashExtended
        @param decayHash output of extraInfo(decayHash)
        @param decayHashExtended output of extraInfo(decayHashExtended)
        """
        # Always avoid the top-level 'import ROOT'.
        import ROOT  # noqa
        return self._forest[ROOT.Belle2.DecayForest.decayHashFloatToInt(decayHash, decayHashExtended)].getOriginalTree()

    def get_reconstructed_decay(self, decayHash, decayHashExtended):
        """
        Return reconstructed DecayTree given the decayHash and decayHashExtended
        @param decayHash output of extraInfo(decayHash)
        @param decayHashExtended output of extraInfo(decayHashExtended)
        """
        # Always avoid the top-level 'import ROOT'.
        import ROOT  # noqa
        return self._forest[ROOT.Belle2.DecayForest.decayHashFloatToInt(decayHash, decayHashExtended)].getReconstructedTree()

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
    except BaseException:
        pass

    if selected:
        if pybasf2.LogPythonInterface.terminal_supports_colors():
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
