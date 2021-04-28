#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import pybasf2
from basf2.utils import print_all_modules, print_params
from basf2.core import set_log_level
from terminal_utils import Pager


def print_module_list(modName=None):
    """
    This wraps around the ``print_all_modules`` function but sanitizes potential command line arguments before doing so.

    Parameters:
      modName: Can be the name of a module or package. Prints more information about a specific module or reduces the output
      to requested package.
    """

    # Do not show INFO messages in module list (actually a problem of the module)
    set_log_level(pybasf2.LogLevel.WARNING)

    # Get the list of available modules
    avModList = pybasf2.list_available_modules()

    if modName is not None:
        # If exactly one argument is given, print the specified module.
        if modName in avModList:
            try:
                current_module = pybasf2._register_module(modName)
                with Pager('Module information for "%s"' % (modName), quit_if_one_screen=True):
                    print_params(current_module, False, avModList[modName])
            except pybasf2.ModuleNotCreatedError:
                pybasf2.B2FATAL('The module could not be loaded.')
            except Exception as e:
                pybasf2.B2FATAL("An exception occured when trying to create the module: %s" % e)

        elif modName == modName.lower():
            # lower case? might be a package instead
            with Pager('List of modules in package "%s"' % (modName)):
                print_all_modules(avModList, modName)
        else:
            pybasf2.B2FATAL('Print module information: A module with the name "' +
                            modName + '" does not exist!')
    else:
        # Otherwise print all modules.
        with Pager('List of all basf2 modules'):
            print_all_modules(avModList)


if __name__ == "__main__":
    try:
        argument = sys.argv[1]
    except IndexError:
        argument = None
    finally:
        print_module_list(argument)
