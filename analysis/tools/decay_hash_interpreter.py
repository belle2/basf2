#!/usr/bin/env python3

import sys
import re
import pdg
import basf2

from pybasf2 import *
# inspect is also used by LogPythonInterface. Do not remove
import inspect
logging = LogPythonInterface()


def parse_logfile(logfile):
    hash2string = dict()
    with open(logfile, 'r') as f:
        for line in f:
            if line.startswith('FOUND_NEW_DECAY_HASH'):
                fields = line.split('|')
                if len(fields) < 4:
                    raise RuntimeError("Error in parsing logfile in line: " + line)
                hash2string[int(fields[1])] = '\n'.join(fields[2:])
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

    entries = entry.split('\n')
    all_particles = re.findall(r"(-?[0-9]+)", entries[0])

    assert len(all_particles) == len(entries) - 2

    table = []
    table.append(["Reconstructed Decay ", prettify_pdg_codes(entries[0])])
    for particle, mc_decay_string in zip(all_particles, entries[1:]):
        table.append([prettify_pdg_codes(particle), prettify_pdg_codes(mc_decay_string)])

    basf2.pretty_print_table(table, column_widths=[25, '*'])

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage {name} basf2.log decayHashInteger'.format(sys.argv[0]))

    logfile = sys.argv[1]
    hash2string = parse_logfile(logfile)

    decayHashInteger = int(sys.argv[2])
    print_hash(hash2string[decayHashInteger])
