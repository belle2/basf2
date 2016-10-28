#!/usr/bin/env python3

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
import root_numpy


def bitwiseConversion(value, i='f', o='i'):
    s = struct.pack('>' + i, value)
    return struct.unpack('>' + o, s)[0]


def decayHashFloatToInt(decayHash, decayHashExtended):
    decayHashInt = bitwiseConversion(np.float32(decayHash))
    decayHashExtendedInt = bitwiseConversion(np.float32(decayHashExtended))
    decayHashFullInt = decayHashInt << 32
    decayHashFullInt += decayHashExtendedInt
    return decayHashFullInt


def parse_rootfile(rootfile):
    ntuple = root_numpy.root2array(rootfile)
    hash2string = {}
    for decayHash, decayHashExtended, decayString in ntuple:
        hash2string[decayHashFloatToInt(decayHash, decayHashExtended)] = decayString
    return hash2string


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
            return '\x1b[32m' + pdg_string + '\x1b[0m'
        else:
            return '^' + pdg_string
    return pdg_string


def prettify_pdg_codes(text):
    text = re.sub(r"(\^?-?[0-9]+)", lambda x: pdg_to_name(x.group(0)), text)
    return text


def print_hash(entry):
    entries = entry.split('|')
    all_particles = re.findall(r"(-?[0-9]+)", entries[0])

    assert len(all_particles) == len(entries) - 1

    table = []
    table.append(["Reconstructed Decay ", prettify_pdg_codes(entries[0])])
    for particle, mc_decay_string in zip(all_particles, entries[1:]):
        table.append([prettify_pdg_codes(particle), prettify_pdg_codes(mc_decay_string)])

    basf2.pretty_print_table(table, column_widths=[25, '*'])

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print('Usage {name} hashmap.root decayHash decayHashExtended'.format(sys.argv[0]))

    rootfile = sys.argv[1]
    hash2string = parse_rootfile(rootfile)

    print_hash(hash2string[decayHashFloatToInt(float(sys.argv[2]), float(sys.argv[3]))])
