#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Skript to create or edit your ipython config to use a port and a password and not open a browser
# when starting.

from jinja2 import Template
from IPython.lib import passwd
import os
from subprocess import check_output


def main():
    '''
    Main function of the script.
    '''
    print("Please fill in the options you want to use for the notebook server.")

    # Ask the user for a password
    password = passwd()

    # Ask the user for a port
    while True:
        try:
            port = int(input('port: '))
        except ValueError:
            print("Please fill in a valid port.")
            continue
        else:
            break

    print("Will now write your notebook config.")

    base_name = os.path.dirname(os.path.realpath(__file__))

    with file(os.path.join(base_name, "jupyter_notebook_config.py.j2"), 'r') as f:
        template = Template(f.read())

        try:
            jupyter_folder = check_output(['jupyter', '--config-dir']).strip()
        except OSError:
            print('Failed to create config file. Have you a recent ipython-notebook installation?')
            raise

        if not os.path.isdir(jupyter_folder):
            check_output(['jupyter', 'notebook', '--generate-config'])

        config_file = template.render(port=port, password=password)

        jupyter_config_file = os.path.join(jupyter_folder, 'jupyter_notebook_config.py')

        # Ask the user whether to override his config
        if os.path.isfile(jupyter_config_file):
            while True:
                choice = input('You already have a jupyter config file. Do you want to replace it? [y/n] ').lower()
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

        with file(jupyter_config_file, 'w') as out:
            out.write(config_file)

        # Set the correct read-write-user-only permissions
        os.chmod(jupyter_config_file, 0o600)

if __name__ == '__main__':
    main()
