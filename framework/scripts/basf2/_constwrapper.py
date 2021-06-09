#!/usr/bin/env python3

"""
Modify the PyDBObj and PyDBArray classes to return read only objects to prevent
accidental changes to the conditions data.

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
        raise ValueError("Object does not have a valid dictionary: %r" % obj)

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


@pythonization(namespace="Belle2")
def _pythonize(klass, name):
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
