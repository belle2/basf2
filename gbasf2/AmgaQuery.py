#!/usr/bin/env python
# -*- coding: utf-8 -*-

# from Config import Config

'''
Created on Jan 16, 2010

@author: milosz
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

###############################################################################

    def __init__(self):
        '''
        Constructor. Connects with AMGA.
        '''

        # connect with amga here
        # self.config = Config(yamlfile)
        self.amgaclient = AmgaClient()

###############################################################################

    def searchQuery(
        self,
        dataType,
        experiments,
        query,
        ):
        '''
        Executes search query.
        '''

        exp = []
        lfns = []

        if experiments is not None:
            for e in experiments:
                exp.append('/belle2/' + dataType + '/E' + str(e) + '/FC:lfn')  # XXX those paths need to be read from config file
        else:
            exp = self.amgaclient.getSubdirectories('/belle2/' + dataType)  # XXX as above
            for i in xrange(len(exp)):
                exp[i] += '/FC'

        for e in exp:
            lfns.extend(self.amgaclient.getGUIDs(e, query))
        return lfns


###############################################################################

