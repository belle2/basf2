#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import ROOT
from ROOT import Belle2
from basf2 import Module


class DataStorePrinter(object):
    """
    Class to print contents of a StroeObjPtr or StoreArray.

    This class is inteded to print the contents of dataobjects to the standard
    output to monitor changes to the contents among versions.

    For example:

    >>> printer = DataStorePrinter("MCParticle", ["getVertex"], {"hasStatus": [1, 2, 4]})
    >>> printer.print()

    will loop over all MCParticle instances in the MCParticles StoreArray and
    print someting like ::

        MCParticle#0
          getVertex(): (0,0,0)
          hasStatus(1): True
          hasStatus(2): False
          hasStatus(4): False

    for each MCparticle
    """

    def __init__(self, name, simple, withArgument=None, array=True):
        """
        Initialize

        Args:
            name (str): class name of the DataStore object
            simple (list): list of member names to print which do not need any additional
                arguments
            withArgument (dict or None): dictionary of member names and a list of
                all argument combinations to call them.
            array (bool): if True we print a StoreArray, otherwise a single StoreObjPtr
        """
        #: class name of the datastore object
        self.name = name
        #: if True we print a StoreArray, otherwise a single StoreObjPtr
        self.array = array
        #: list of object members to call and print their results
        self.object_members = []

        # add the simple members to the list of members to call with empty
        # arguments
        for member in simple:
            self.object_members.append((member, [], None, None))

        # and add the members with Argument
        if withArgument:
            for member, arguments in withArgument.items():
                for args in arguments:
                    # convert args to a list
                    if not isinstance(args, list) or isinstance(args, tuple):
                        args = [args]
                    # and add (name,args,display,callback) tuple
                    self.object_members.append((member, args, None, None))

        # sort them to have fixed order
        self.object_members.sort(key=lambda x: repr(x))

    def add_member(self, name, arguments=[], callback=None, display=None):
        """
        Add an additional member to be printed.

        Args:
            name (str): name of the member
            arguments (list): arguments to pass to the member when calling it
            callback (function or None): callback function to print the result
                of the member call. The function will be called with the arguments
                (name, arguments, result)
            display (str or None): display string to use when printing member call
                info instead of function name and arguments
        """
        bisect.insort(self.object_members, (name, arguments, callback, display))
        # return self for method chaining
        return self

    def print(self):
        """Print all the objects currently existing"""
        if self.array:
            data = Belle2.PyStoreArray(self.name + "s")
            for i, obj in enumerate(data):
                self._printObj(obj, i)
        else:
            obj = Belle2.PyStoreObj(self.name)
            if obj:
                self._printObj(obj.obj())

    def _printObj(self, obj, index=None):
        """Print all defined members for each object with given index.
        If we print a StoreObjPtr then index is None and this function is called
        once. If we print StoreArrays this function is called once for each
        entry in the array with index set to the position in the array
        """
        # print array index? If yes then add it to the output
        if index is not None:
            index = "#%d" % index
        else:
            index = ""

        print("%s%s:" % (self.name, index))

        # loop over all defined member/argument combinations
        # and print "member(arguments): result" for each
        for name, args, callback, display in self.object_members:
            result = getattr(obj, name)(*args)
            # display can be used to override what is printed for the member. If
            # so, use it here
            if display is not None:
                print("  " + display + ": ", end="")
            else:
                # otherwise just print name and arguments
                print("  %s(%s): " % (name, ",".join(map(str, args))), end="")
            # if a callback is set the callback is used to print the result
            if callback is not None:
                sys.stdout.flush()
                callback(name, args, result)
            # otherwise use default function
            else:
                self._printResult(result)

    def _printResult(self, result, depth=0, weight=None):
        """ Print the result of calling a certain member.
        As we want the test to be independent of memory we have to be a bit careful
        how to not just print() but check whether the object is maybe a pointer to another
        DataStore object or if it is a TObject with implements Print().
        Also, call recursively std::pair

        Args:
            result: object to print
            depth (int): depth for recursive printing, controls the level of indent
            weight (float or None): weight to print in addition to object, only used for
            relations
        """
        # are we in recursion? if so indent the output
        if depth:
            print("  " * (depth + 1), end="")

        if weight is not None:
            weight = " (weight: %.6g)" % weight
        else:
            weight = ""

        # is it another RelationsObject? print array name and index
        if hasattr(result, "getArrayName") and hasattr(result, "getArrayIndex"):
            if not result:
                print("-> NULL%s" % weight)
            else:
                print("-> %s#%d%s" % (result.getArrayName(), result.getArrayIndex(), weight))
        # special case for TMatrix like types to make them more space efficient
        elif hasattr(result, "GetNrows") and hasattr(result, "GetNcols"):
            print(weight, end="")
            for row in range(result.GetNrows()):
                print("\n" + "  " * (depth + 2), end="")
                for col in range(result.GetNcols()):
                    print("%13.6e " % result(row, col), end="")

            print()
        # or, does it look like a std::pair?
        elif hasattr(result, "first") and hasattr(result, "second"):
            print("pair%s" % weight)
            self._printResult(result.first, depth + 1)
            self._printResult(result.second, depth + 1)
        # or, could it be a std::vector like container?
        elif hasattr(result, "size") and hasattr(result, "begin") and hasattr(result, "end"):
            print("size(%d)%s" % (result.size(), weight))
            # if it is a RelationVector we also want to print the weights. So
            # check whether we have weights and pass them to the _printResult
            weight_getter = getattr(result, "weight", None)
            weight = None
            # loop over all elements and print the elements with one level more
            # indentation
            for i, e in enumerate(result):
                if weight_getter is not None:
                    weight = weight_getter(i)
                self._printResult(e, depth + 1, weight=weight)
        # print floats with 6 valid digits
        elif isinstance(result, float):
            print("%.6g%s" % (result, weight))
        # ok, in any case we can just print it
        else:
            print(result, weight, sep="")


# ok, now we just need a small class to call all the printer objects for each
# event
class PrintObjectsModule(Module):

    """Call all DataStorePrinter objects in for each event"""

    def __init__(self, objects_to_print):
        self.objects_to_print = objects_to_print
        super().__init__()

    def event(self):
        """print the contents of the mdst mdst_dataobjects"""
        for printer in self.objects_to_print:
            printer.print()
