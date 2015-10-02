#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

import sys

from ROOT import Belle2

import argparse

parser = \
    argparse.ArgumentParser(description='Manually adds a payload to the conditions database.',
                            usage='basf2 %(prog)s [option1] [option2]'
                            )

parser.add_argument('--payload',
                    metavar='Payload (i.e.  /path/to/ConditionsPayload.root)',
                    required=True,
                    help='The path & filename for the payload to be added.  This parameter is REQUIRED.'
                    )

parser.add_argument(
    '--package',
    metavar='Package (i.e.  topcaf, cdc, or pxd)',
    required=True,
    help='The BASF2 package the payload is associated with.  This parameter is REQUIRED and must be a registered BASF2 package.')

parser.add_argument(
    '--module',
    metavar='Module (i.e.  Pedestal)',
    required=True,
    help='The BASF2 module the payload is associated with.  This parameter is REQUIRED,' +
         ' but does not need to be a recognized BASF2 module.')

parser.add_argument('--comment',
                    metavar='Comment (i.e.  \'my favorite payload\')',
                    required=False,
                    help='A comment to associate with the payload.  This parameter is OPTIONAL.'
                    )

parser.add_argument('--RESTbase',
                    metavar='RESTbase (i.e.  http://belle2db.hep.pnnl.gov/b2s/rest/v1/)',
                    required=False,
                    help='The URL for the conditions DB service.  This parameter is OPTIONAL.'
                    )

args = parser.parse_args()


conditions = register_module('Conditions')
conditionsDict = {'restBaseName': 'http://belle2db.hep.pnnl.gov/b2s/rest/v1/',
                  'fileName': args.payload,
                  'package': args.package,
                  'module': args.module}

conditions.param(conditionsDict)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [2])

main.add_module(eventinfosetter)
main.add_module(conditions)

process(main)
