##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from sphinx import addnodes
from sphinx.roles import XRefRole
from sphinx.directives import ObjectDescription
from sphinx.domains import Domain, ObjType, Index
from sphinx.util.docfields import TypedField, Field
from sphinx.util.nodes import make_refnode
from sphinx.util import logging
logger = logging.getLogger(__name__)


class Basf2Object(ObjectDescription):
    """
    A basf2 x-ref directive registered with Sphinx.add_object_type().
    Basically copied together from sphinxcontrib-domaintools and sphinxcontrib-adadomain.
    """

#: \cond Doxygen_suppress

    doc_field_types = {
        TypedField("parameter", label="Parameters", names=("param", "parameter", "arg", "argument"),
                   typenames=("paramtype", "type"), typerolename=None, can_collapse=True),
        Field('returnvalue', label='Returns', has_arg=False,
              names=('returns', 'return')),
    }

    def handle_signature(self, sig, signode):
        signode.clear()

        # check if we have arguments
        try:
            name, args = sig.split("(", 1)
        except ValueError:
            name = sig
            args = None

        # Add the name of the signature
        name = name.strip()
        signode += addnodes.desc_name(name, name)

        # if there was a opening parenthesis add all the arguments to the
        # parameter list, separate them by comma
        if args:
            paramlist = addnodes.desc_parameterlist()
            for arg in (e.strip() for e in args[:-1].split(",")):
                if not arg:
                    continue
                paramlist += addnodes.desc_parameter(arg, arg)
            signode += paramlist

        # return the object name for referencing
        return name

#: \endcond

    def add_target_and_index(self, name, sig, signode):
        # Create a full id from objtype and name
        targetname = f'{self.objtype}-{name}'
        # and append the id to the object node
        signode['ids'].append(targetname)
        self.state.document.note_explicit_target(signode)

        # remember the name -> (document, targetname) mapping in the domain data
        # dictionary so that we can use it to resolve x-refs
        ddata = self.env.domaindata["b2"][self.objtype + 's']

        if name in ddata:
            # already exists, give warning
            logger.warn(f"Duplicate description of basf2 {self.objtype} {name}, " +
                        "Other instance in " + self.env.doc2path(ddata[name][0]) +
                        ", please add ':noindex:' to one",
                        location=(self.env.docname, self.lineno))
        else:
            ddata[name] = (self.env.docname, targetname)


class Basf2ModuleIndex(Index):
    """Create an alphabetic index of all modules"""
    #: \cond Doxygen_suppress
    name = "modindex"
    localname = "basf2 Module Index"
    shortname = "basf2 modules"

    def generate(self, docnames=None):
        content = {}
        modules = self.domain.data["modules"].items()
        for modname, (docname, target) in sorted(modules):
            letter = modname[0].upper()
            content.setdefault(letter, [])
            content[letter].append([modname, 0, docname, target, "", "", ""])
        return list(content.items()), False
    #: \endcond


class Basf2VariableIndex(Index):
    """Create an alphabetic index of all variables"""
    #: \cond Doxygen_suppress
    name = "varindex"
    localname = "basf2 Variable Index"
    shortname = "basf2 variables"

    def generate(self, docnames=None):
        content = {}
        modules = self.domain.data["variables"].items()
        modules = sorted(modules, key=lambda x: x[0].lower())
        for modname, (docname, target) in modules:
            letter = modname[0].upper()
            content.setdefault(letter, [])
            content[letter].append([modname, 0, docname, target, "", "", ""])
        return list(content.items()), False
    #: \endcond


class Basf2Domain(Domain):
    """basf2 Software Domain"""
    #: \cond Doxygen_suppress
    name = "b2"
    label = "Belle II Software"
    object_types = {
        "module": ObjType("module", "mod"),
        "variable": ObjType("variable", "var")
    }

    directives = {
        "module": Basf2Object,
        "variable": Basf2Object,
    }
    roles = {
        "mod": XRefRole(),
        "var": XRefRole(),
    }
    initial_data = {
        "modules": {},
        "variables": {},
    }
    indices = [
        Basf2ModuleIndex,
        Basf2VariableIndex,
    ]
    #: \endcond

    def clear_doc(self, docname):
        """Remove the existing domain data for a given document name"""
        for t in "modules", "variables":
            try:
                for name, (doc, target) in list(self.data[t].items()):
                    if doc == docname:
                        del self.data[t][name]
            except Exception:
                pass

    #: \cond Doxygen_suppress
    def get_objects(self):
        for i, type in enumerate(["modules", "variables"]):
            for name, (docname, target) in self.data[type].items():
                yield(name, name, type, docname, target, i)

    def get_type_name(self, type, primary=False):
        # never prepend "Default"
        return type.lname

    def resolve_xref(self, env, fromdocname, builder,
                     typ, target, node, contnode):
        t = {"mod": "modules", "var": "variables"}[typ]
        try:
            docname, labelid = self.data[t][target]
            return make_refnode(builder, fromdocname, docname,
                                labelid, contnode)
        except Exception:
            return None
    #: \endcond
