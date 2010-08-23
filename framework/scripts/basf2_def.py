from basf2_env import *

#-----------------------------------------------
#              Register module
#
#  name: The name of the module type
#-----------------------------------------------
def register_module( name ):
  return fw.register_module(name)


#-----------------------------------------------
#                Create path
#
#-----------------------------------------------
def create_path():
  return fw.create_path()


#-----------------------------------------------
#                Process events
#
#  path: The path with which the processing starts
#-----------------------------------------------
def process( path ):
  fw.process(path)


#-----------------------------------------------
#                Process events
#
#  path:      The path with which the processing starts
#  max_event: The max_event number of events which will be processed
#-----------------------------------------------
def process( path, max_event ):
  fw.process(path, max_event)


#-----------------------------------------------
#                Process events
#
#  path:       The path with which the processing starts
#  max_event:  The max_event number of events which will be processed
#  run_number: The run number
#-----------------------------------------------
def process( path, max_event, run_number ):
  fw.process(path, max_event, run_number)


#-----------------------------------------------
#        Print parameter information
#
#  module: Print the param info of this module
#-----------------------------------------------
def print_params( module ):
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

