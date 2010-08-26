from basf2_env import *

def register_module( name ):
    """"
    This function registers a new module

    name: The name of the module type
    """
    return fw.register_module(name)


def create_path():
    """
    This function creates a new path
    """
    return fw.create_path()


def process( path, max_event=0, run_number=None ):
    """
    This function processes the events

    path: The path with which the processing starts
    max_event:  The max_event number of events which will be processed
    run_number: The run number
    """
    if run_number is not None:
        fw.process(path, max_event, run_number)
    else:
        fw.process(path, max_event)


def print_params( module ):
    """
    This function prints parameter information

    module: Print the parameter information of this module
    """

    print ""
    print "==================="
    print '%-20s %s' % (module.type(), module.description())
    print "-------------------"
    print '%-20s %-20s %-20s %-30s %-10s %s' % ("Name", "Type", "Default", "Current", "Steering", "Description")
    print "-------------------"

    paramList = module.available_params()
    for paramItem in paramList:
        defaultStr = ", ".join(['%s' % defaultItem for defaultItem in paramItem.default])
        valueStr   = ", ".join(['%s' % valueItem for valueItem in paramItem.values])
        print '%-20s %-20s %-20s %-30s %-10s %s' % (paramItem.name, paramItem.type, defaultStr, valueStr, paramItem.setInSteering, paramItem.description)
    print ""

