##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import subprocess
import shlex


def call_command(command):
    """Call command print output after having removed some lines from output

    Args:
        command (string): command to be called
    """
    output = subprocess.check_output(shlex.split(command), encoding='utf-8').strip().split('\n')
    return output


if __name__ == '__main__':
    output = call_command('b2help-decay print 1111440100')
    assert len(output) > 10
    output = call_command('b2help-decay print 1111440100.dec')
    assert len(output) > 10
    output = call_command('b2help-decay find K_S0 J/psi B0')
    assert len(output) > 10
    output = call_command('b2help-decay find K_S0 J/psi -m B0 -b')
    assert len(output) > 2
