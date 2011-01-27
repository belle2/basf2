#!/usr/bin/env python
# This script is used to generate a ganga job file
#using a template job and a list of queues
# Tom Fifield 2009-10
queues_file = 'queues.txt'

queues = open(queues_file, 'r')

for queue in queues:
 print "#Ganga# Job object (category: jobs)\n\
 Job (\n\
 name = '' ,\n\
 outputsandbox = [] ,\n\
 inputdata = None ,\n\
 merger = None ,\n\
 inputsandbox = [ ] ,\n\
 application = Executable (\n\
    exe = 'echo' ,\n\
    env = {} ,\n\
    args = ['Hello World'] \n\
    ) ,\n\
 outputdata = None ,\n\
 splitter = None ,\n\
 backend = LCG (\n\
    CE = '" + queue.rstrip() +"',\n\
    middleware = 'gLite'\n\
    )  )"
 
