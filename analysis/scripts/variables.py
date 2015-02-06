from basf2 import *
from ROOT import gSystem
gSystem.Load('libanalysis.so')

# import everything into current namespace.
from ROOT import Belle2
variables = Belle2.Variable.Manager.Instance()


def printVars():
    """
    Print list of all available variables.
    """

    print 'Available variables in Variable::Manager:'
    allVars = variables.getVariables()
    rows = []
    group = ''
    for v in allVars:
        if v.group != group:
            group = v.group
            rows.append([group])
        rows.append([v.name, v.description])
    pretty_print_description_list(rows)


if __name__ == "__main__":
    printVars()
