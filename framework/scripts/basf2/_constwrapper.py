#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Modify the PyDBObj and PyDBArray classes to return read only objects to prevent
accidental changes to the conditions data.
Also modify the functions fillArray and readArray of the PyStoreArray class to
ensure a safe and simple usage of those function.

This module does not contain any public functions or variables, it just
modifies the ROOT interfaces for PyDBObj and PyDBArray to only return read only
objects.
"""

# we need to patch PyDBObj so lets import the ROOT cppyy backend.
import cppyy

# However importing ROOT.kIsConstMethod and kIsStatic is a bad idea here since
# it triggers final setup of ROOT and thus starts a gui thread (probably).
# So we take them as literal values from TDictionary.h. We do have a unit test
# to make sure they don't change silently though.

#: EProperty::kIsPublic value from TDictionary.h
_ROOT_kIsPublic = 0x00000200
#: EProperty::kIsStatic value from TDictionary.h
_ROOT_kIsStatic = 0x00004000
#: EProperty::kIsConstMethod value from TDictionary.h
_ROOT_kIsConstMethod = 0x10000000


# copy and enhanced version of ROOT.pythonization
def pythonization(lazy=True, namespace=""):
    """
    Pythonizor decorator to be used in pythonization modules for pythonizations.
    These pythonizations functions are invoked upon usage of the class.
    Parameters
    ----------
    lazy : boolean
        If lazy is true, the class is pythonized upon first usage, otherwise
        upon import of the ROOT module.
    """
    def pythonization_impl(fn):
        """
        The real decorator. This structure is adopted to deal with parameters
        fn : function
            Function that implements some pythonization.
            The function must accept two parameters: the class
            to be pythonized and the name of that class.
        """
        if lazy:
            cppyy.py.add_pythonization(fn, namespace)
        else:
            fn()
    return pythonization_impl


def _make_tobject_const(obj):
    """
    Make a TObject const: Disallow setting any attributes and calling any
    methods which are not declared as const. This affects any reference to this
    object in python and stays permanent. Once called this particular instance
    will be readonly everywhere and will remain like this.

    This is done by replacing all setter functions with function objects that
    just raise an attribute error when called. The class will be still the same
    """
    # nothing to do if None
    if obj is None:
        return obj

    try:
        #: list of all non-const, non-static, public methods
        non_const = [m.GetName() for m in obj.Class().GetListOfMethods() if (m.Property() & _ROOT_kIsPublic)
                     and not (m.Property() & (_ROOT_kIsConstMethod | _ROOT_kIsStatic))]
    except AttributeError:
        raise ValueError(f"Object does not have a valid dictionary: {obj!r}")

    # Override all setters to just raise an exception
    for name in non_const:
        def __proxy(*args, **argk):
            """raise attribute error when called"""
            raise AttributeError(f"{obj} is readonly and method '{name}' is not const")

        setattr(obj, name, __proxy)

    # and return the modified object
    return obj


def _PyDBArray__iter__(self):
    """Provide iteration over Belle2::PyDBArray. Needs to be done here since we
    want to make all returned classes read only"""
    for i in range(len(self)):
        yield self[i]


numpy_to_cpp = {"int16": "short *",
                "uint16": "unsigned short *",
                "int32": "int *",
                "uint32": "unsigned int *",
                "int64": "long *",
                "uint64": "unsigned long *",
                "float32": "float *",
                "float64": "double *",
                "float96": "long double *"}

cpp_to_numpy = {"short": "short",
                "unsigned short": "ushort",
                "int": "intc",
                "unsigned int": "uintc",
                "long": "long",
                "unsigned long": "ulong",
                "float": "single",
                "double": "double",
                "long double": "longdouble",
                "Belle2::VxdID": "ushort"}


class ConstructorNotFoundError(Exception):
    '''
    Class that throws an exception when a specific constructor is not found.
    The error message will contain the signature of the wanted constructor,
    as well as the signature of the available constructors.
    '''
    def __init__(self, members, list_constructors, obj_name):
        '''
        Parameters:
        -----------
        members: dictionary
            Contains the name of the parameters of the wanted constructor and their types

        list_constructors: list of dictionaries
            Contains all the available constructors, with the names of their parameters
            and their types

        obj_name: string
            Name of the class of which the constructor is wanted
        '''
        #: Member contatining the final message of the exception
        self.message = "No corresponding constructor found. Looking for signature: \n"
        #: Dictionary
        self.members = members
        #: List of constructors
        self.list_constructors = list_constructors
        #: Name of the class
        self.name = obj_name
        self.message = self.message + self.name + "("
        self.message = (self.message +
                        ", ".join([" ".join(i) for i in list(zip(self.members.values(), self.members.keys()))]) +
                        ")\n Available constructors:\n")
        for lis in self.list_constructors:
            self.message = (self.message +
                            self.name + "(" + ", ".join([" ".join(i) for i in list(zip(lis.values(), lis.keys()))]) + ")\n")
        super().__init__(self.message)


def _wrap_fill_array(func):

    def fill_array(pyStoreArray, **kwargs):
        import numpy as np

        list_constructors = []

        obj_class = pyStoreArray.getClass()
        obj_classname = obj_class.GetName()

        for meth in obj_class.GetListOfMethods():
            if meth.GetName() == obj_classname.split(":")[-1]:  # found one constructor
                d = {}

                for ar in meth.GetListOfMethodArgs():
                    d[ar.GetName()] = ar.GetTypeName()
                list_constructors.append(d)

                # Check if this is the right constructor
                if d.keys() == kwargs.keys():
                    break

        else:
            m_d = {list(kwargs.keys())[i]: numpy_to_cpp[type(list(kwargs.values())[i]
                                                             [0]).__name__].split("*")[0] for i in range(len(kwargs.keys()))}
            raise ConstructorNotFoundError(m_d, list_constructors, obj_classname)

        arr_types = []
        for k in kwargs.keys():
            if kwargs[k][0].dtype != np.dtype(cpp_to_numpy[d[k]]):
                try:
                    kwargs[k] = kwargs[k].astype(cpp_to_numpy[d[k]])
                except ValueError:
                    raise ValueError((f"Impossible to convert type of input arrays ({type(kwargs[k][0]).__name__})" +
                                      f" into the type of the corresponding class member '{k}' ({np.dtype(cpp_to_numpy[d[k]])})"))
            arr_types.append(numpy_to_cpp[type(kwargs[k][0]).__name__])

        l_arr = list(kwargs.values())
        if not all(len(l_arr[0]) == len(arr) for arr in l_arr[1:]):
            raise ValueError("The lengths of the passed arrays are not the same")

        length = len(l_arr[0])

        func[(obj_classname, *arr_types)](pyStoreArray, length, *[kwargs[k] for k in d.keys()])

    return fill_array


def _wrap_read_array(func):

    def read_array(pyStoreArray):
        import numpy as np
        kwargs = {}

        obj_class = pyStoreArray.getClass()
        obj_classname = obj_class.GetName()

        fillValuesMethod = obj_class.GetMethodAny("fillValues")
        if not fillValuesMethod:
            raise ReferenceError(f"The method fillValues is not implemented for the class {obj_classname}")

        for ar in fillValuesMethod.GetListOfMethodArgs():
            kwargs.setdefault(ar.GetName(), np.zeros(len(pyStoreArray), dtype=cpp_to_numpy[ar.GetFullTypeName().split("*")[0]]))

        l_types = [numpy_to_cpp[kwargs[v].dtype.name] for v in kwargs.keys()]

        func[(obj_classname, *l_types)](pyStoreArray, *kwargs.values())

        return kwargs

    return read_array


@pythonization(namespace="Belle2")
def _pythonize(klass, name):
    """Adjust the python interface of some Py* classes"""
    if not name.startswith("Py"):
        return

    if name == "PyDBObj":
        # now replace the PyDBObj getter with one that returns non-modifiable objects.
        # This is how root does it in ROOT.py so let's keep that
        klass.obj = lambda self: _make_tobject_const(self._obj())
        # and allow to use it most of the time without calling obj() like the ->
        # indirection in C++
        klass.__getattr__ = lambda self, name: getattr(self.obj(), name)
        # and make sure that we can iterate over the items in the class
        # pointed to if it allows iteration
        klass.__iter__ = lambda self: iter(self.obj())
    elif name == "PyDBArray":
        # also make item access in DBArray readonly
        klass.__getitem__ = lambda self, i: _make_tobject_const(self._get(i))
        # and supply an iterator
        klass.__iter__ = _PyDBArray__iter__
    elif name == "PyStoreObj":
        # make sure that we can iterate over the items in the class
        # pointed to if it allows iteration
        klass.__iter__ = lambda self: iter(self.obj())
    elif name == "PyStoreArray":
        klass.fillArray = _wrap_fill_array(klass.fillArray)
        klass.readArray = _wrap_read_array(klass.readArray)
