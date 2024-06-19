##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import argparse
import basf2.utils as b2utils


class PythonVariableManager(object):
    '''
    Wrapper around the variable manager class.
    This is necessary for avoiding to import ROOT globally when 'variables' is imported.
    '''

    @classmethod
    def _instance(self):
        '''
        Return an instance to the variable manager.
        '''
        # Always avoid the top-level 'import ROOT'.
        import ROOT  # noqa
        instance = ROOT.Belle2.Variable.Manager.Instance()
        return instance

    def getVariable(self, *args):
        '''
        Wrapper around Manager::getVariable(std::string name) and
        Manager::getVariable(const std::string& functionName, const std::vector<std::string>& functionArguments).
        '''
        instance = PythonVariableManager._instance()
        return instance.getVariable(*args)

    def getVariables(self, *args):
        '''
        Wrapper around Manager::getVariables(const std::vector<std::string>& variables) and
        Manager::getVariables().
        '''
        instance = PythonVariableManager._instance()
        return instance.getVariables(*args)

    def addAlias(self, alias, variable):
        '''
        Wrapper around Manager::addAlias(const std::string& alias, const std::string& variable).
        '''
        instance = PythonVariableManager._instance()
        assert(instance.addAlias(alias, variable))

    def printAliases(self):
        '''
        Wrapper around Manager::printAliases().
        '''
        instance = PythonVariableManager._instance()
        instance.printAliases()

    def clearAliases(self):
        '''
        Wrapper around Manager::clearAliases().
        '''
        instance = PythonVariableManager._instance()
        instance.clearAliases()

    def resolveAlias(self, alias):
        '''
        Wrapper around Manager::resolveAlias(const std::string& alias).
        '''
        instance = PythonVariableManager._instance()
        return instance.resolveAlias(alias)

    def addCollection(self, *args):
        '''
        Wrapper around Manager::addCollection(const std::string& collection, const std::vector<std::string>& variables)
        '''
        instance = PythonVariableManager._instance()
        assert(instance.addCollection(*args))

    def getCollection(self, collection):
        '''
        Wrapper around Manager::getCollection(const std::string& collection).
        '''
        instance = PythonVariableManager._instance()
        return instance.getCollection(collection)

    def resolveCollections(self, *args):
        '''
        Wrapper around Manager::resolveCollections(const std::vector<std::string>& variables).
        '''
        instance = PythonVariableManager._instance()
        return instance.resolveCollections(*args)

    def checkDeprecatedVariable(self, variable):
        '''
        Wrapper around Manager::checkDeprecatedVariable(const std::string& name).
        '''
        instance = PythonVariableManager._instance()
        instance.checkDeprecatedVariable(variable)

    def evaluate(self, variable, particle):
        '''
        Wrapper around Manager::evaluate(const std::string& varName, const Particle* p).
        '''
        instance = PythonVariableManager._instance()
        return instance.evaluate(variable, particle)

    def getNames(self):
        '''
        Wrapper around Manager::getNames().
        '''
        instance = PythonVariableManager._instance()
        return instance.getNames()

    def getAliasNames(self):
        '''
        Wrapper around Manager::getAliasNames().
        '''
        instance = PythonVariableManager._instance()
        return instance.getAliasNames()

    def assertValidName(self, variable):
        '''
        Wrapper around Manager::assertValidName(const std::string& name).
        '''
        instance = PythonVariableManager._instance()
        instance.assertValidName(variable)


#: Allow users to easily interact with the variable mananger.
variables = PythonVariableManager()


def std_vector(*args):
    """
    Creates an std::vector which can be passed to pyROOT
    """
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    v = ROOT.std.vector(type(args[0]))()
    for x in args:
        v.push_back(x)
    return v


def getCommandLineOptions():
    """ Parses the command line options and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('--no-pager', dest='pager', default=True, action='store_false',
                        help='Use a pager to show output or print to terminal.')
    args = parser.parse_args()
    return args


def printVars(changedVariableNames=None):
    """
    Print list of all available variables.
    """

    if changedVariableNames:
        print(changedVariableNames)
    print('Available variables in Variable::Manager:')
    allVars = variables.getVariables()
    vars = []
    for v in allVars:
        vars.append((v.group, v.name, v.description))

    rows = []
    current_group = ''
    for (group, name, description) in sorted(vars):
        if current_group != group:
            current_group = group
            rows.append([group])
        rows.append([name, description])
    b2utils.pretty_print_description_list(rows)


def getAllTrgNames():
    """
    Return all PSNM trigger bit names
    """
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    bits = ROOT.Belle2.PyDBObj('TRGGDLDBFTDLBits')
    evt = ROOT.Belle2.EventMetaData()
    ROOT.Belle2.DBStore.Instance().update(evt)
    size = ROOT.Belle2.TRGSummary.c_ntrgWords * ROOT.Belle2.TRGSummary.c_trgWordSize
    return [bits.getoutbitname(i) for i in range(size) if bits.getoutbitname(i) != '']
