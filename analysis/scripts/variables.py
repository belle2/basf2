from ROOT import gSystem
gSystem.Load('libanalysis.so')

#import everything into current namespace.
from ROOT import Belle2
variables = Belle2.Variable.Manager.Instance()

print 80 * '='
print 'Available variables in Variable::Manager:'
print 80 * '='
variables.printList()
print 80 * '='
