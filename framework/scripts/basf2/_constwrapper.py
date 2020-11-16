#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Modify the PyDBObj and PyDBArray classes to return read only objects to prevent
accidental changes to the conditions data.

This module does not contain any public functions or variables, it just
modifies the ROOT interfaces for PyDBObj and PyDBArray to only return read only
objects.
"""


# we need to patch PyDBObj so lets import the ROOT cpp backend.
import cppyy as _cppyy
# However importing ROOT.kIsConstMethod and kIsStatic is a bad idea
# here since it triggers final setup of ROOT and thus starts a gui thread
# (probably) and consumes command lines if not disabled *sigh*. So we take them
# as literal values from TDictionary.h. We do have a unit test to make sure
# they don't change silently though.

#: EProperty::kIsStatic value from TDictionary.h
_ROOT_kIsStatic = 0x00004000
#: EProperty::kIsConstMethod value from TDictionary.h
_ROOT_kIsConstMethod = 0x10000000


class _TObjectConstWrapper:
    """Empty class to check whether an instance is already const wrapped"""


def _make_tobject_nonconst(obj):
    """Make a once read only TObject writeable again"""
    if isinstance(obj, _TObjectConstWrapper):
        object.__setattr__(obj, "__class__", obj.__real_class__)
        del obj.__real_class__
    return obj


def _make_tobject_const(obj):
    """
    Make a TObject const: Disallow setting any attributes and calling any
    methods which are not declared as const. This affects any reference to this
    object in python and stays permanent. Once called this particular instance
    will be readonly everywhere and will remain like this.

    This is done by modifying the __class__ attribute of the object and replace
    it with a new subclass which hides all non-const, non-static member
    functions and disable attribute setting. The reason we do it like this is
    that this is the only way how comparison and copy constructor calls work
    transparently. E.g with a normal instance of type T 'a' and a const wrapped
    T 'b' we can do things like a == b, b == a, a < b, b > a, c = T(b).
    """
    # nothing to do if already const wrapped or None
    if obj is None or isinstance(obj, _TObjectConstWrapper):
        return obj

    # ROOT 6.14/04 issues a runtime warning for GetListOfAllPublicMethods() but
    # the code still seems to work fine (ROOT-9699)
    import warnings
    try:
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            #: list of all non-const, public methods
            non_const = [m.GetName() for m in obj.Class().GetListOfAllPublicMethods()
                         if not (m.Property() & (_ROOT_kIsConstMethod | _ROOT_kIsStatic))]
    except AttributeError:
        raise ValueError("Object does not have a valid dictionary: %r" % obj)

    def __make_const_proxy(obj, name):
        """return a proxy function which just raises an attribute error on access"""

        def proxy(self, *args):
            """raise attribute error when called"""
            raise AttributeError("%s is readonly and method '%s' is not const" % (obj, name))
        return proxy

    def __setattr(self, name, value):
        """disallow setting of any attributes"""
        raise AttributeError("%s is readonly, can't set attribute" % obj)

    # ok we create a derived class which removes access to non-const, non-static member
    # functions by adding properties for them which will always cause
    # AttributeErrors
    scope = {m: property(__make_const_proxy(obj, m)) for m in non_const}
    # Also add a __setattr__ to make sure nobody changes anything
    scope["__setattr__"] = __setattr
    # create a dynamic type for this which inherits from the original type and
    # _TObjectConstWrapper so we can distuingish it later. Add all our proxies
    # as scope
    normal_type = type(obj)
    const_type = type("const %s" % normal_type.__name__, (normal_type, _TObjectConstWrapper), scope)
    # remember the old class
    obj.__real_class__ = normal_type
    # and sneakily replace the class of this object :D
    obj.__class__ = const_type
    # done
    return obj


def _PyDBArray__iter__(self):
    """Provide iteration over Belle2::PyDBArray. Needs to be done here since we
    want to make all returned classes read only"""
    for i in range(len(self)):
        yield self[i]


# now replace the PyDBObj getter with one that returns non-modifiable objects.
# This is how root does it in ROOT.py so let's keep that
_dbobj_scope = _cppyy._backend.CreateScopeProxy("Belle2::PyDBObj")
_dbobj_scope.obj = lambda self: _make_tobject_const(self._obj())
# and allow to use it most of the time without calling obj() like the ->
# indirection in C++
_dbobj_scope.__getattr__ = lambda self, name: getattr(self.obj(), name)
# also make item access in DBArray readonly
_dbarray_scope = _cppyy._backend.CreateScopeProxy("Belle2::PyDBArray")
_dbarray_scope.__getitem__ = lambda self, i: _make_tobject_const(self._get(i))
# and supply an iterator
_dbarray_scope.__iter__ = _PyDBArray__iter__
# and make sure that if we can iterate over the items in the class pointed to
# by the StoreObjPtr or DBObjPtr it allows iteration
_dbobj_scope.__iter__ = lambda self: iter(self.obj())
_storeobj_scope = _cppyy._backend.CreateScopeProxy("Belle2::PyStoreObj")
_storeobj_scope.__iter__ = lambda self: iter(self.obj())
