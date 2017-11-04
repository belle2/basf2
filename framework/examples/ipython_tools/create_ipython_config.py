#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Skript to create or edit your ipython config to use a port and not open a browser window
# when starting.
# Please ensure you have the newest jupyter notebook version installed (greater/equal than 4.0.0).
#

from jinja2 import Template
import os
from subprocess import check_output
from ROOT import Belle2


def main():
    '''
    Main function of the script.
    '''
    print("Please fill in the options you want to use for the notebook server.")

    # Ask the user for a port
    while True:
        try:
            port = int(input('Network Port (the recommendation is a number between 8000 to 9000): '))
        except ValueError:
            print("Please fill in a valid network port.")
            continue
        else:
            break

    print("Will now write your notebook config.")

    jupyter_template_file = Belle2.FileSystem.findFile("framework/examples/ipython_tools/jupyter_notebook_config.py.j2")
    with open(jupyter_template_file, 'r') as f:
        template = Template(f.read())

        try:
            jupyter_folder = check_output(['jupyter', '--config-dir']).decode().strip()
        except OSError:
            print('Failed to create config file. Have you a recent ipython-notebook installation?')
            raise

        if not os.path.isdir(jupyter_folder):
            try:
                check_output(['jupyter', 'notebook', '--generate-config'])
            except BaseException:
                print("Could not start jupyter notebook. There are many possible reasons for this.\n"
                      "Please see https://confluence.desy.de/display/BI/Software+Jupyter+Notebooks for possible fixes \n"
                      "and feel free to contact software@belle2.org for questions.")

        config_file = template.render(port=port)
        jupyter_config_file = os.path.join(jupyter_folder, 'jupyter_notebook_config.py')

        # Ask the user whether to override his config
        if os.path.isfile(jupyter_config_file):
            while True:
                choice = input('You already have a jupyter config file. Do you want to replace it? [Y/n] ').lower()
                if choice == "n":
                    print('Not writing config file.')
                    exit()
                if choice == 'y' or choice == '':
                    print('Overwriting config file.')
                    break
                else:
                    print('Not a valid answer.')
                    continue
        else:
            print('Writing config file.')

        with open(jupyter_config_file, 'w') as out:
            out.write(config_file)

        # Set the correct read-write-user-only permissions
        os.chmod(jupyter_config_file, 0o600)


if __name__ == '__main__':
    main()
