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


DecayNode = collections.namedtuple('DecayNode', "p,d,m")
DecayNode.__new__.__defaults__ = (0, None, False)
DecayNode.__doc__ = """
    DecayNode describes the decay of a particle p, into
    list of daughters d. In addition it can have list of connections
    or match m to another DecayNode (e.g. between MC and reconstructed tree)
    """


class DecayTree(object):
    """
    Consists of a tree of DecayNodes.
    Can be constructed from the output of the ParticleMCDecayString module
    """
    def __init__(self, decaystring):
        """
        Builds a new DecayTree using a decaystring.
        The string should look like this:
         300553 (--> -521 (--> 15 (--> 11)) 521 (--> -423 (--> -421 (--> 321 -211) 111 (--> 22 22)) -13))
        @param decaystring part of the output of the ParticleMCDecayString
        """
        #: List of decay nodes in the order of their appearance in the decay string for fast access
        self.particles = []
        #: Decay nodes of the tree, for a well formed decaystring the list should have only one node
        self.nodes = []
        if 'No match' in decaystring:
            self.nodes = [DecayNode()]
        else:
            decaylist = decaystring.replace('-->', '').replace(')', ' ) ').replace('(', ' ( ').replace('^', '').split(' ')
            decaylist = [x.strip() for x in decaylist]
            _, self.nodes = self.build_tree(decaylist)

    def build_tree(self, decaylist, i=0):
        """
        Recursively build a new tree
        @param decaylist linear list of tokens from the decaystring
        @param i position of current token
        """
        nodes = []
        while i < len(decaylist):
            x = decaylist[i]
            if x:
                if x == '(':
                    i, d = self.build_tree(decaylist, i+1)
                    nodes[-1].d.extend(d)
                elif x == ')':
                    return i, nodes
                else:
                    nodes.append(DecayNode(int(x), [], []))
                    self.particles.append(nodes[-1])
            i += 1
        return i, nodes

    def nodes_match(self, node1, node2, unmatched=False):
        """
        Check if two DecayNodes are compatible.
        To be compatible they must:
        - Have the same mother particle p
        - Have the same number of daughters particles or one of the daughter particles list have to be None
        - The daughter particles have to fullfill the same conditions pairwise
        In particular the order auf the daughter particles matter!
          423 (--> 421 13) is not the same as 423 (--> 13 421)
        @param node1 first node
        @param node2 second node
        @param unmatched enforce the DecayNode which is found to be not matched
        """
        if node1.p == node2.p:
            if node1.d is None or node2.d is None:
                if node1.d is None or node2.d is None:
                    if unmatched:
                        return node1.m is False
                return True
            if len(node1.d) != len(node2.d):
                return False
            if all([self.nodes_match(subnode1, subnode2) for subnode1, subnode2 in zip(node1.d, node2.d)]):
                if unmatched:
                    return node1.m is False
                return True
        return False

    def find_decay(self, decay, nodes=None, unmatched=False):
        """
        Check if the decay tree contains the given decay tree.
        @param decay DecayTree or DecayNode object describing the decay
        @param nodes used to call the method recursively
        @param unmatched enforce the DecayNode which is found to be not matched
        """
        if isinstance(decay, DecayTree):
            if len(decay.nodes) != 1:
                raise RuntimeError("Given decay tree does not contain exactly one decay!")
            decay = decay.nodes[0]
        if nodes is None:
            nodes = self.nodes
        for node in nodes:
            if self.nodes_match(decay, node, unmatched):
                return True
            elif self.find_decay(decay, node.d, unmatched):
                return True
        return False

    def print_node(self, node, indent=0):
        """
        Recirsively print DecayNode with some indent
        """
        output = '  ' * indent
        output += str(node.p) + '\n'
        for d in node.d:
            output += self.print_node(d, indent + 1)
        return output

    def __str__(self):
        """
        Convert DecayTree to a string
        """
        output = ''
        for node in self.nodes:
            output += self.print_node(node) + '\n'
        return output


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
    DecayHashMap contains all the DecayTrees for all decays written out
    by the ParticleMCDecayString module
    """
    def __init__(self, rootfile):
        """
        Create a new DecayHashMap using a root file
        @param rootfile a ROOT file outputted by the ParticleMCDecayString module
        """
        import root_numpy
        ntuple = root_numpy.root2array(rootfile)
        #: Dict Int -> DecayStrings
        self._string = {}
        #: Dict Int -> Reconstructed DecayTree
        self._reconstructed_decay = {}
        #: Dict Int -> Original (MC) Decay Tree
        self._original_decay = {}
        for decayHash, decayHashExtended, decayString in ntuple:
            decayInt = _decayHashFloatToInt(decayHash, decayHashExtended)
            if decayInt in self._string:
                continue
            self._string[decayInt] = decayString
            splitted = decayString.split('|')
            self._reconstructed_decay[decayInt] = DecayTree(splitted[0].strip())
            first_valid = [x for x in splitted[1:] if 'No match' not in x][0]
            self._original_decay[decayInt] = DecayTree(first_valid.strip())
            for j, line in enumerate(splitted[1:]):
                tokens = line.replace('(--> ', '').replace(')', '').replace('(', '').split(' ')
                matched = [k for k, x in enumerate(tokens) if x.startswith('^')]
                if (len(matched) == 1 and
                   len(self._reconstructed_decay[decayInt].particles) > j and
                   len(self._original_decay[decayInt].particles) > matched[0]):
                    k = matched[0]
                    self._reconstructed_decay[decayInt].particles[j].m.append(self._original_decay[decayInt].particles[k])
                    self._original_decay[decayInt].particles[k].m.append(self._reconstructed_decay[decayInt].particles[j])

    def get_string(self, decayHash, decayHashExtended):
        """
        Return DecayString given the decayHash and decayHashExtended
        @param decayHash output of extraInfo(decayHash)
        @param decayHashExtended output of extraInfo(decayHashExtended)
        """
        return self._string[_decayHashFloatToInt(decayHash, decayHashExtended)]

    def get_original_decays(self, decayHash, decayHashExtended):
        """
        Return original (MC) DecayTree given the decayHash and decayHashExtended
        @param decayHash output of extraInfo(decayHash)
        @param decayHashExtended output of extraInfo(decayHashExtended)
        """
        return self._original_decay[_decayHashFloatToInt(decayHash, decayHashExtended)]

    def get_reconstructed_decay(self, decayHash, decayHashExtended):
        """
        Return reconstructed DecayTree given the decayHash and decayHashExtended
        @param decayHash output of extraInfo(decayHash)
        @param decayHashExtended output of extraInfo(decayHashExtended)
        """
        return self._reconstructed_decay[_decayHashFloatToInt(decayHash, decayHashExtended)]

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
