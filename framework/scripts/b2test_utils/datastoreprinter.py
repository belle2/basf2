#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2, kIsPublic, kIsStatic, TVector3, TLorentzVector
from basf2 import Module, B2FATAL


def get_public_members(classname):
    """
    Return a list of public, non-static member functions for a given classname.
    The class must exist in the Belle2 namespace and have a ROOT dictionary
    """
    tclass = getattr(Belle2, classname).Class()
    members = {e.GetName() for e in tclass.GetListOfMethods()
               if (e.Property() & kIsPublic) and not (e.Property() & kIsStatic)}

    # filter known members from ClassDef and constructor/destructor
    # Remove some Dictionary members
    removed = {
        "CheckTObjectHashConsistency", "Class", "Class_Name", "Class_Version",
        "DeclFileLine", "DeclFileName", "Dictionary", "ImplFileLine",
        "ImplFileName", "IsA", "ShowMembers", "Streamer", "StreamerNVirtual",
        "operator!=", "operator=", "operator==",
        # we don't need constructor and destructor either
        classname, f"~{classname}",
    }
    members -= removed
    return list(sorted(members))


class DataStorePrinter:
    """
    Class to print contents of a StoreObjPtr or StoreArray.

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

        # sort them by member name to have fixed order: python sort is
        # guaranteed to be stable so different calls to the same member will
        # remain in same relative order
        self.object_members.sort(key=lambda x: x[0])

    def add_member(self, name, arguments=None, print_callback=None, display=None):
        """
        Add an additional member to be printed.

        Args:
            name (str): name of the member
            arguments (list or callable): arguments to pass to the member when calling it
                If this is a callable object then the function is called with
                the object as first argument and the member name to be tested as
                second argument. The function is supposed to return the list of
                arguments to pass to the member when calling. Possible return
                valus for the callable are:

                * a `list` of arguments to be passed to the member. An empty
                  `list` means to call the member with no arguments.
                * a `tuple` of `lists <list>` to call the member once for each
                  list of arguments in the tuple
                * `None` or an empty tuple to not call the member at all
            print_callback (function or None): if not None a function to print
                the result of the member call. The function will be called with
                the arguments (name, arguments, result) and should print the
                result on stdout without any additional information.
            display (str or None): display string to use when printing member call
                info instead of function name + arguments. If it is not given
                the default output will be ``{membername}({arguments}):``
                followed by the result.
        """
        if arguments is None:
            arguments = []
        self.object_members.append((name, arguments, print_callback, display))
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

    def print_untested(self):
        """Print all the public member functions we will not test"""
        members = get_public_members(self.name)
        tested = {e[0] for e in self.object_members}
        for member in members:
            if member in tested:
                continue
            print(f"Untested method {self.name}::{member}")

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

        print(f"{self.name}{index}:")

        # loop over all defined member/argument combinations
        # and print "member(arguments): result" for each
        for name, arguments, callback, display in self.object_members:
            # if arguments is callable it means we need to call it to determine
            # the arguments
            if callable(arguments):
                all_args = arguments(obj, name)
                # None means we don't calle the member this time
                if all_args is None:
                    continue
                # list is one set of arguments, tuple(list) is n set of
                # arguments. So convert list into tuple of length 1 to call
                # member once
                if isinstance(all_args, list):
                    all_args = (all_args,)
            else:
                # if arguments is not callable we asume it's one set of
                # arguments
                all_args = (arguments,)

            for args in all_args:
                result = getattr(obj, name)(*args)
                # display can be used to override what is printed for the member. If
                # so, use it here
                if display is not None:
                    print("  " + display + ": ", end="")
                else:
                    # otherwise just print name and arguments
                    print("  {}({}): ".format(name, ",".join(map(str, args))), end="")
                # if a callback is set the callback is used to print the result
                if callback is not None:
                    print("", end="", flush=True)
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
        # or is it a TVector3 or TLorentzVector?
        elif isinstance(result, TVector3):
            print("(" + ",".join("%.6g" % result[i] for i in range(3)) + ")")
        elif isinstance(result, TLorentzVector):
            print("(" + ",".join("%.6g" % result[i] for i in range(4)) + ")")
        # or, does it look like a std::pair?
        elif hasattr(result, "first") and hasattr(result, "second"):
            print("pair%s" % weight)
            self._printResult(result.first, depth + 1)
            self._printResult(result.second, depth + 1)
        # or, could it be a std::vector like container? But ROOT might wrap a std::string so if it has npos assume it's a string
        elif (hasattr(result, "size") and hasattr(result, "begin") and hasattr(result, "end")) and not hasattr(result, "npos"):
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
            print(f"{result:.6g}{weight}")
        # print char as int
        elif isinstance(result, str) and len(result) == 1:
            print(ord(result), weight, sep="")
        # ok, in any case we can just print it
        else:
            print(result, weight, sep="")


# ok, now we just need a small class to call all the printer objects for each
# event
class PrintObjectsModule(Module):

    """Call all DataStorePrinter objects in for each event"""

    def __init__(self, objects_to_print, print_untested=False):
        """
        Initialize

        Args:
            objects_to_print (list): list of object to print
        """
        #: list of object to print
        self.objects_to_print = objects_to_print
        #: print untested members?
        self.print_untested = print_untested
        super().__init__()

    def initialize(self):
        """Print all untested members if requested"""
        if not self.print_untested:
            return

        for printer in self.objects_to_print:
            printer.print_untested()

    def event(self):
        """print the contents of the mdst mdst_dataobjects"""
        try:
            for printer in self.objects_to_print:
                printer.print()
        except Exception as e:
            B2FATAL("Error in datastore printer: ", e)
