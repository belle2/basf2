##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Helper functions for producing events with the WHIZARD generator.
'''

import hashlib
import os
import shutil
import subprocess

import b2test_utils
import basf2
import beamparameters


#: Default number of iterations for WHIZARD integration
_default_iterations = '25:10000:"gw", 10:10000:""'

#: Refined number of iterations for WHIZARD integration
_refined_iterations = '25:10000:"gw", 20:10000:""'

#: Dictionary for WHIZARD: key is process name, value is a dictionary with final_state, cuts and iterations
_processes_dict = {
    'eeee': {
        'final_state': '"e-", "e+", "e-", "e+"',
        'cuts': 'cuts = all M > 500 MeV ["e-" + "e+"]',
        'iterations': _refined_iterations,
    },
    'eemumu': {
        'final_state': '"e-", "e+", "mu-", "mu+"',
        'cuts': 'cuts = all M > 500 MeV ["mu+" + "mu-"]',
        'iterations': _default_iterations
    },
    'eetautau': {
        'final_state': '"e-", "e+", "tau-", "tau+"',
        'cuts': '',
        'iterations': _default_iterations
    },
    'mumumumu': {
        'final_state': '"mu-", "mu+", "mu-", "mu+"',
        'cuts': '',
        'iterations': _default_iterations
    },
    'mumutautau': {
        'final_state': '"mu-", "mu+", "tau-", "tau+"',
        'cuts': '',
        'iterations': _default_iterations
    },
    'tautautautau': {
        'final_state': '"tau-", "tau+", "tau-", "tau+"',
        'cuts': '',
        'iterations': _default_iterations
    }
}

#: Template of SINDARIN file for the supported processes
_sindarin_template = '''
process {process} = "e-", "e+" => {final_state}

! ISR configuration
beams =  "e-", "e+" => isr
?isr_handler = true
$isr_handler_mode = "recoil"
?keep_beams = true
?keep_remnants = true
isr_mass = me

seed = {random_seed}
sqrts = {cm_energy} GeV
n_events = {events}
sample_format = lhef
{cuts}
integrate ({process}) {{
          iterations = {iterations}
          relative_error_goal = 0.05
}}
simulate ({process})
'''


def get_sindarin(process, events, cm_energy, random_seed=114):
    '''
    Return a properly formatted SINDARIN file as a string.
    '''
    if process not in _processes_dict.keys():
        basf2.B2FATAL(f'The process {process} is currently not supported, you need to write a SINDARIN file yourself.')
    sindarin = _sindarin_template.format(
        process=process,
        final_state=_processes_dict[process]['final_state'],
        cuts=_processes_dict[process]['cuts'],
        iterations=_processes_dict[process]['iterations'],
        events=events,
        cm_energy=cm_energy,
        random_seed=random_seed
    )
    return sindarin


def run_whizard(process, experiment, run, events, print_sindarin=False):
    '''
    This function takes care of running WHIZARD.
    '''
    if process not in _processes_dict.keys():
        basf2.B2FATAL(f'The process {process} is currently not supported, you need to write a SINDARIN file yourself.')

    cm_energy = beamparameters.get_collisions_invariant_mass(experiment, run)

    basf2_seed = basf2.get_random_seed().encode('utf-8')
    whizard_seed = int(hashlib.sha256(basf2_seed).hexdigest(), 16) % 10**8

    sindarin = get_sindarin(process, events, cm_energy, whizard_seed)
    if print_sindarin:
        basf2.B2INFO(f'The SINDARIN file is:\n\n{sindarin}')

    cwd = os.getcwd()
    path = os.path.join(cwd, f'{process}_{whizard_seed}')
    whizard = 'whizard'
    sin = f'{process}.sin'
    lhe = f'{process}.lhe'
    log = f'{process}.log'

    if not os.path.exists(path):
        os.mkdir(path)
    else:
        shutil.rmtree(path, ignore_errors=True)
        os.mkdir(path)

    # Hacky solution for keeping track if WHIZARD failed or not
    # since we want to ensure that the working directory is actually cleaned
    failed = False

    # Run WHIZARD dumping the output files in a temporary directory
    with b2test_utils.clean_working_directory():

        with open(sin, 'w') as sindarin_file:
            sindarin_file.write(sindarin)

        tmp = os.getcwd()

        try:
            subprocess.check_call([whizard, '-L', log, sin])
            moved_lhe, moved_log = os.path.join(path, lhe), os.path.join(path, log)
            shutil.move(os.path.join(tmp, lhe), moved_lhe)
            shutil.move(os.path.join(tmp, log), moved_log)
        except subprocess.CalledProcessError:
            failed = True

    if failed:
        basf2.B2FATAL(f'WHIZARD failed while generating the {process} process')

    return path, moved_lhe, moved_log


if __name__ == "__main__":
    pass
