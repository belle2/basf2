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


class DecayTree(object):
    def __init__(self, decaystring):
        self.particles = []
        if 'No match' in decaystring:
            self.nodes = [DecayNode()]
        else:
            decaylist = decaystring.replace('-->', '').replace(')', ' ) ').replace('(', ' ( ').replace('^', '').split(' ')
            decaylist = [x.strip() for x in decaylist]
            _, self.nodes = self.build_tree(decaylist)

    def build_tree(self, decaylist, i=0):
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

    def nodes_match(self, node1, node2):
        if node1.p == node2.p:
            if node1.d is None or node2.d is None:
                return True
            if len(node1.d) != len(node2.d):
                return False
            if all([self.nodes_match(subnode1, subnode2) for subnode1, subnode2 in zip(node1.d, node2.d)]):
                return True
        return False

    def find_decay(self, decay, nodes=None):
        if nodes is None:
            nodes = self.nodes
        for node in nodes:
            if self.nodes_match(decay, node):
                return True
            elif self.find_decay(decay, node.d):
                return True
        return False

    def print_node(self, node, indent=0):
        output = '  ' * indent
        output += str(node.p) + '\n'
        for d in node.d:
            output += self.print_node(d, indent + 1)
        return output

    def __str__(self):
        output = ''
        for node in self.nodes:
            output += self.print_node(node) + '\n'
        return output


def _bitwiseConversion(value, i='f', o='i'):
    s = struct.pack('>' + i, value)
    return struct.unpack('>' + o, s)[0]


def _decayHashFloatToInt(decayHash, decayHashExtended):
    decayHashInt = _bitwiseConversion(np.float32(decayHash))
    decayHashExtendedInt = _bitwiseConversion(np.float32(decayHashExtended))
    decayHashFullInt = decayHashInt << 32
    decayHashFullInt += decayHashExtendedInt
    return decayHashFullInt


class DecayHashMap(object):
    def __init__(self, rootfile):
        import root_numpy
        ntuple = root_numpy.root2array(rootfile)
        self._string = {}
        self._reconstructed_decay = {}
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
        return self._string[_decayHashFloatToInt(decayHash, decayHashExtended)]

    def get_original_decays(self, decayHash, decayHashExtended):
        return self._original_decay[_decayHashFloatToInt(decayHash, decayHashExtended)]

    def get_reconstructed_decay(self, decayHash, decayHashExtended):
        return self._reconstructed_decay[_decayHashFloatToInt(decayHash, decayHashExtended)]

    def print_hash(self, decayHash, decayHashExtended):
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


def pdg_to_name(x):
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
    text = re.sub(r"(\^?-?[0-9]+)", lambda x: pdg_to_name(x.group(0)), text)
    text = text.replace('gamma', 'g').replace('--> ', '').replace('anti-', 'a-')
    text = text.replace('Upsilon', 'Y').replace(') ', ')').replace(' (', '(')
    return text
