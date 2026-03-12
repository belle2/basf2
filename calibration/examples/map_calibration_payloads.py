##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
A very simple example that prints the list of payloads produced by each prompt calibration.
'''

import importlib
from pathlib import Path

import basf2
from prompt import prompt_script_package, prompt_script_dir


def get_script_settings(name):
    cal_module = importlib.import_module(prompt_script_package + Path(name).stem)
    return cal_module.settings


all_script_paths = Path(basf2.find_file(prompt_script_dir)).glob("*.py")
all_script_paths = [p.name for p in all_script_paths]

payload_names = []
for script_path in all_script_paths:
    settings = get_script_settings(script_path)
    print(f'Calibration: {settings.name} -> Payloads: {settings.produced_payloads}')
    payload_names.extend(settings.produced_payloads)

print('\nAll the payloads produced during the prompt calibration are:\n', payload_names)
