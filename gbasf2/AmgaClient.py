#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
Created on Jan 17, 2010

@author: milosz
'''

import mdclient
import sys

sys.path.append('../')

# from cnfg import Config


class AmgaClient(object):

    '''
    Module to handle connection with AMGA metadata-base
    '''

    config = None
    client = None

###############################################################################

    def __init__(self):
        '''
        Constructor - needs path of yaml file with configuration
        '''

        # self.config = Config.Config(confile)
        self.client = mdclient.MDClient('150.183.246.196', 8822, 'belle_user',
                'belle')

###############################################################################

    def getSubdirectories(self, dir):
        '''
        Get subdirectories in metadata catalog.
        '''

        result = []
        self.client.execute('ls ' + dir)
        while not self.client.eot():
            result.append(dir + '/' + self.client.fetchRow().split('/')[-1])
        return result

###############################################################################

    def getGUIDs(self, experiment, query):
        '''
        Old way of getting LFNs filfilling query.
        '''

        guids = []
        self.client.selectAttr([experiment], query)
        while not self.client.eot():
            # guids.append('guid:' + self.client.fetchRow())
            guids.append(self.client.fetchRow())
        return guids

###############################################################################

    def directQuery(self, experiment, query):
        '''
        Query with ability to define parameters in SQL way.
        Returns list of LFNs.
        '''

        self.client.find('*', query)
        results = []
        while not self.client.eot():
            results.append(self.client.fetchRow())
        return results


###############################################################################
