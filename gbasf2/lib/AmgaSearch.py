#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# AmgaSearch is a container for information used in creating
# a search-style metadata query to AMGA
#

'''
Created on Jan 17, 2010

@author: milosz, fifieldt
'''

from AmgaQuery import AmgaQuery
from AmgaClient import AmgaClient


class AmgaSearch(object):

    '''
    Class for searching AMGA base to find LFNs of files with given parameters.
    '''

    dataType = 'data'
    lfns = None
    experiments = None
    attributes = None
    queryString = None
    project = None
    username = None

###############################################################################

    def __init__(self):
        '''
        Constructor - initializes variables
        '''

        self.lfns = set()
        self.experiments = set()
        self.attributes = []

###############################################################################

    def setDataType(self, dataType):
        '''
        Set data type for searching:
            - 'data'
            - 'MC'
        '''

        if dataType == 'MC':  # XXX just for now
            dataType = 'MC/generic'  # XXX just for now
        self.dataType = dataType

###############################################################################

    def setExperiments(self, experiments):
        '''
        Set experiment number to search in. If nothing is set, all experiments will be used.
        Argument is list of experiment numbers, i.e. [12, 31, 65]
        '''

        if type(experiments) is not list:
            self.experiments = []
            # XXX should throw something here... We have to discuss exceptions policy.
        self.experiments = experiments

###############################################################################

    def setAttributesWithOperators(
        self,
        attributes,
        operators,
        values,
        ):
        '''
        Prepare attributes array
        Arguments:
        - list of attributes, i.e. ['runL', 'runH']
        - list of operators, i.e. ['>', '<']
        - list of values to compare, i.e. [10, 100]
        '''

        self.attributes = []
        for i in xrange(len(operators)):
            if operators[i] == 'gt':
                operators[i] = '>'
            if operators[i] == 'lt':
                operators[i] = '<'
            if operators[i] == 'ge':
                operators[i] = '>='
            if operators[i] == 'le':
                operators[i] = '<='

        for (attr, op, val) in zip(attributes, operators, values):
            self.attributes.append((attr, op, val))

###############################################################################

    def setAttributes(self, attributes):
        '''
       Sets a list of attributes to be returned with the Search
      '''

        self.attributes = attributes

###############################################################################

    def setUserData(self, project):
        '''Set project name for the search'''

        self.project = project

###############################################################################

    def setUsername(self, username):
        '''Set username for the search'''

        self.username = username

###############################################################################

    def getDataType(self):
        return self.dataType

    def getLfns(self):
        return self.lfns

    def getExperiments(self):
        return self.experiments

    def getAttributes(self):
        return self.attributes

    def getQueryString(self):
        return self.queryString

###############################################################################

    def executeAmgaQuery(self):
        '''
        Execute AMGA query and return list of LFNs
        No arguments
        '''

        if len(self.queryString) == 0:
            raise Exception('No query. Aborting.')

        aq = AmgaQuery()
        results = aq.searchQuery(self.dataType, self.experiments,
                                 self.queryString)
        return results

###############################################################################

    def executeAmgaQueryWithAttributes(self):
        '''
        Execute AMGA query and return list of LFNs
        No arguments
        '''

        if len(self.queryString) == 0:
            raise Exception('No query. Aborting.')

        aq = AmgaQuery()
        results = aq.searchQueryWithAttributes(
            dataType=self.dataType,
            experiments=self.experiments,
            query=self.queryString,
            attributes=self.attributes,
            project=self.project,
            username=self.username,
            )
        return results

###############################################################################

    def setQuery(self, query):
        self.queryString = query


###############################################################################

# if used not as a module: make tests, show workflow
if __name__ == '__main__':
    asearch = AmgaSearch()
    # asearch.setDataType('data')  # or setDataType('MC')
    asearch.setDataType('user')  # or setDataType('MC')
    asearch.setExperiments(['07', '09', '35'])
    # asearch.setQuery('(id > 1 and id < 5) or events = 666')
    # asearch.setQuery('True')
    asearch.setQuery('2>1')
    asearch.setAttributes(['lfn', 'events'])
    asearch.setUsername('hanyl')
    asearch.setUserData('creation8129')
    # lfns = asearch.executeAmgaQuery()
    results = asearch.executeAmgaQueryWithAttributes()
    print results
