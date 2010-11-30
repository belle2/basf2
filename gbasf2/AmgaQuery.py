#!/usr/bin/env python
# -*- coding: utf-8 -*-

# from Config import Config

'''
Created on Jan 16, 2010

@author: milosz, fifieldt
'''

from AmgaClient import AmgaClient


class AmgaQuery(object):

    '''
    Class querying AMGA metadata-base and returning results.
    '''

    results = []
    query = ''
    amgaclient = None
    config = None
    vo = 'belle2'

###############################################################################

    def __init__(self):
        '''
        Constructor. Connects with AMGA.
        '''

        # connect with amga here
        self.amgaclient = AmgaClient()

###############################################################################

    def searchQuery(
        self,
        dataType,
        experiments,
        query,
        ):
        '''
        Executes search query. Called by AmgaSearch module.
        Arguments:
            - dataType - type of data ('data' or 'MC')
            - experiments - list of numbers of experiments to search in or None to search all
            - query - SQL-like query with definition of data parameters
        '''

        exp = []
        lfns = []

        if experiments is not None:
            for e in experiments:
                exp.append('/' + self.vo + '/' + dataType + '/E' + str(e)
                           + '/FC')  # XXX those paths need to be read from config file
        else:
            exp = self.amgaclient.getSubdirectories('/' + self.vo + '/'
                    + dataType)  # XXX as above
            for i in xrange(len(exp)):
                exp[i] += '/FC'

        for e in exp:
            lfns.extend(self.amgaclient.directQuery(e, query))
        print lfns
        return lfns

###############################################################################

    def searchQueryWithAttributes(
        self,
        dataType,
        experiments,
        query,
        attributes,
        ):
        '''
        Executes search query. Called by AmgaSearch module.
        Arguments:
            - dataType - type of data ('data' or 'MC')
            - experiments - list of numbers of experiments to search in or None to search all
            - query - SQL-like query with definition of data parameters
            - attributes - Attributes to be returned with the query
        '''

        exp = []
        results = {}

        if experiments is not None:
            for e in experiments:
                exp.append('/' + self.vo + '/' + dataType + '/E' + str(e)
                           + '/FC')
        else:
            exp = self.amgaclient.getSubdirectories('/' + self.vo + '/'
                    + dataType)
            for i in xrange(len(exp)):
                exp[i] += '/FC'

        for e in exp:
            results.update(self.amgaclient.directQueryWithAttributes(e, query,
                           attributes))
        print results
        return results


###############################################################################


def registerQuery(self, dataType, metadata):
    '''
  Registers data into AMGA
  input metadata is of form metadata[experiment][entry_id] = ([attributes],[values])
  '''

    for experiment in metadata.keys():
        self.amgaclient.bulkInsert(self, '/' + self.vo + '/' + dataType + '/'
                                   + experiment, metadata[experiment])


