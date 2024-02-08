import os
import shutil
import subprocess

import basf2 as b2
import b2test_utils as b2tu

import random
import string

import argparse

"""
Usage:
basf2 -n <last_event_number> --skip-events <first_event_number>
      --exp <exp_number> --run <run_number> whizard_for_production.py
      -o <output_file_name>.root -- --process <process: eemumu, mumumumu, mumutautau, ...>
"""

# parser


def arg_parser():
    """
    Argument parser
    """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--process',
                        dest='process',
                        default='eemumu',
                        help='final state to generate: eeee, eemumu, eetautau, mumumumu, ...',
                        metavar='PROCESS')
    return parser

# generate random string for file names


def get_random_string(length: int):
    """
    Generate random string with combination of lower and upper case

    :param lenght: number of charachters composing the random string
    """
    result_str = ''.join(random.choice(string.ascii_letters) for i in range(length))

    return result_str

# generate process in sindarin


def process_string_sindarin(process: str):
    """
    Generate process in sindarin for whizard

    :param process: final state to generate
    """
    her = 'e-'
    ler = 'e+'
    base_string = f'process {process} = "{her}", "{ler}" => '
    lst = [*process]
    add_string = ''
    for i in range(len(lst)):
        if lst[i] == 'm':
            lst[i] = "".join([lst[i], lst[i+1]])
        if lst[i] == 't':
            lst[i] = "".join([lst[i], lst[i+1], lst[i+2]])

    lst = [j for i, j in enumerate(lst) if j != 'u']
    lst = [j for i, j in enumerate(lst) if j != 'a']

    if (len(lst) == 2):
        add_string = f'"{lst[0]}-", "{lst[1]}+"'
    elif (len(lst) == 4):
        add_string = f'"{lst[0]}-", "{lst[1]}+", "{lst[2]}-", "{lst[3]}+"'
    else:
        b2.B2FATAL("only 2-lepton or 4-lepton final states are allowed")

    return base_string + add_string

# generate sindarin file needed for production


def generate_sindarin(n_events: int, process: str, path: str):
    """
    Generate configuration file with the right number of events to generate

    :param n_events: number of events
    :param process: final state to generate
    """
    sindarin = os.path.join(path, f'{process}.sin')
    configuration_file = open(sindarin, 'w')
    configuration_file.write(process_string_sindarin(process)+"\n")
    configuration_file.write('beams =  "e-", "e+" => isr\n')
    configuration_file.write('sqrts = 10.579558 GeV\n')
    configuration_file.write(f'n_events = {n_events}\n')
    configuration_file.write('sample_format = lhef\n')
    configuration_file.write('?keep_beams = true\n')  # generate ISR photon
    configuration_file.write('?keep_remnants = true\n')  # generate ISR photon
    configuration_file.write(f'integrate ({process})\n')
    configuration_file.write(f'simulate ({process})')

    configuration_file.close()

    return sindarin


# main
if __name__ == '__main__':

    from ROOT import Belle2

    # number of events to generate
    last = Belle2.Environment.Instance().getNumberEventsOverride()  # -n
    first = Belle2.Environment.Instance().getSkipEventsOverride()  # --skip-events

    n_events = last - first

    # Change the process accordingly
    args = arg_parser().parse_args()
    _process = str(args.process)

    # ### WHIZARD ###
    random_string = get_random_string(8)

    cwd = os.getcwd()
    path = os.path.join(cwd, f'{_process}_lhe_files_{random_string}')
    whizard = 'whizard'
    lhe = f'{_process}.lhe'
    log = f'{_process}.log'

    if not os.path.exists(path):
        os.mkdir(path)
    else:
        shutil.rmtree(path, ignore_errors=True)
        os.mkdir(path)

    sindarin = generate_sindarin(n_events, _process, path)

    # Run WHIZARD dumping the output files in a temporary directory
    with b2tu.clean_working_directory():
        tmp = os.getcwd()
        result = subprocess.run([whizard, '-L', log, sindarin])
        moved_lhe = os.path.join(path, lhe)
        shutil.move(os.path.join(tmp, lhe), moved_lhe)
        shutil.move(os.path.join(tmp, log), os.path.join(path, log))

    # ### basf2 ###

    main = b2.Path()

    main.add_module('EventInfoSetter')

    main.add_module('LHEInput',
                    inputFileList=moved_lhe,
                    makeMaster=False,
                    nInitialParticles=2,
                    nVirtualParticles=0,
                    wrongSignPz=False)

    main.add_module('BoostMCParticles')

    main.add_module('SmearPrimaryVertex')

    main.add_module('Progress')

    main.add_module('RootOutput')

    # main.add_module('PrintMCParticles',
    #                 onlyPrimaries=False,
    #                 showMomenta=True,
    #                 showProperties=True,
    #                 showStatus=True,
    #                 showVertices=True)

    b2.process(main)
    print(b2.statistics)
