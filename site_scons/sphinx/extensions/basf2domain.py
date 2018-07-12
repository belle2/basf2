from sphinx import addnodes
from sphinx.roles import XRefRole
from sphinx.directives import ObjectDescription
from sphinx.domains import Domain, ObjType, Index
from sphinx.util.docfields import TypedField, Field
from sphinx.util.nodes import make_refnode


class Basf2Object(ObjectDescription):
    """
    A Basf2 x-ref directive registered with Sphinx.add_object_type().
    Basically copied together from sphinxcontrib-domaintools and sphinxcontrib-adadomain.
    """

    doc_field_types = {
        TypedField("parameter", label="Parameters", names=("param", "parameter", "arg", "argument"),
                   typenames=("paramtype", "type"), typerolename=None, can_collapse=True),
        Field('returnvalue', label='Returns', has_arg=False,
              names=('returns', 'return')),
    }

    def handle_signature(self, sig, signode):
        signode.clear()

        # check fi we have arguments
        try:
            name, args = sig.split("(", 1)
        except Exception as e:
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

    def add_target_and_index(self, name, sig, signode):
        # Create a full id from objtype and name
        targetname = '%s-%s' % (self.objtype, name)
        # and append the id to the object node
        signode['ids'].append(targetname)
        self.state.document.note_explicit_target(signode)

        # remember the name -> (document, targetname) mapping in the domain data
        # dictionary so that we can use it to resolve x-refs
        ddata = self.env.domaindata["b2"][self.objtype + 's']

        if name in ddata:
            # already exists, give warning
            self.env.warn(self.env.docname,
                          "Duplicate description of basf2 %s %s, " % (self.objtype, name) +
                          "Other instance in " + self.env.doc2path(ddata[name][0]),
                          self.lineno)
        else:
            ddata[name] = (self.env.docname, targetname)


class Basf2ModuleIndex(Index):
    """Create an alphabetic index of all modules"""
    name = "modindex"
    localname = "Basf2 Module Index"
    shortname = "basf2 modules"

    def generate(self, docnames=None):
        content = {}
        modules = self.domain.data["modules"].items()
        for modname, (docname, target) in sorted(modules):
            letter = modname[0].upper()
            content.setdefault(letter, [])
            content[letter].append([modname, 0, docname, target, "", "", ""])
        return content.items(), False


class Basf2VariableIndex(Index):
    """Create an alphabetic index of all variables"""
    name = "varindex"
    localname = "Basf2 Variable Index"
    shortname = "basf2 variables"

    def generate(self, docnames=None):
        content = {}
        modules = self.domain.data["variables"].items()
        for modname, (docname, target) in modules:
            letter = modname[0].upper()
            content.setdefault(letter, [])
            content[letter].append([modname, 0, docname, target, "", "", ""])
        return content.items(), False


class Basf2Domain(Domain):
    """Basf2 Software Domain"""
    name = "b2"
    label = "Belle 2 Software"
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

    def clear_doc(self, docname):
        """Remove the existing domain data for a given document name"""
        for t in "modules", "variables":
            try:
                for name, (doc, target) in list(self.data[t].items()):
                    if doc == docname:
                        del self.data[t][name]
            except Exception:
                pass

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
