import basf2

import os
import os.path
import sys
import argparse
import collections


class ArgumentParser(argparse.ArgumentParser):

    """An argparse.Argument parse slightly changed such
    that it always prints an extended help message incase of a parsing error."""

    def error(self, message):
        """Method invoked when a parsing error occured.
        Writes an extended help over the base ArgumentParser.
        """
        self.print_help()
        sys.stderr.write('error: %s\n' % message)
        sys.exit(2)


class NonstrictChoices(list):

    """Class that instances can be given to an argparse.ArgumentParser.add_argument as choices keyword argument.

    The explicit choices stated during construction of this object are just suggestions but all other values are
    excepted as well.
    """

    def __contains__(self, value):
        """Test for correctness of the choices.
        Always returns true since all choices should be valid not only the ones stated at construction of this object.
        """
        return True

    def __iter__(self):
        """Displays all explicit values and a final "..." to indicate more choices might be possible."""
        # Append an ellipses to indicate that there are more choices.
        copy = list(super().__iter__())
        copy.append('...')
        return iter(copy)

    def __str__(self):
        """Displays all explicit values and a final "..." to indicate more choices might be possible."""
        # Append an ellipses to indicate that there are more choices.
        copy = list(self)
        copy.append('...')
        return str(copy)


def find_file(file_path):
    # dec_file_path = Belle2.FileSystem.findFile(generator_module)
    belle2_local_dir = os.environ.get("BELLE2_LOCAL_DIR", None)
    if belle2_local_dir:
        local_file_path = os.path.join(belle2_local_dir, file_path)
        if os.path.exists(local_file_path) and os.path.isfile(local_file_path):
            return local_file_path

    belle2_central_dir = os.environ.get("BELLE2_RELEASE_DIR", None)
    if belle2_central_dir:
        central_file_path = os.path.join(belle2_central_dir, file_path)
        if os.path.exists(central_file_path) and os.path.isfile(central_file_path):
            return central_file_path

    if os.path.exists(file_path) and os.path.isfile(file_path):
        return file_path

    return None


def extend_path(path,
                module,
                module_by_short_name={},
                allow_function_import=False):
    """Convenience adder function that can resolve addtional short hand module names from a dictionary"""
    if isinstance(module, basf2.Module):
        # A module instance
        path.add_module(module)

    elif isinstance(module, str):
        # A module name of a short name as defined in the forwarded dictionary
        if allow_function_import:
            if "." in module:
                # Allow imports from the local directory
                sys.path.append(os.getcwd())

                py_module_name, function_name = module.rsplit(".", 1)
                try:
                    import importlib
                    py_module = importlib.import_module(py_module_name)
                except ImportError:
                    pass
                else:
                    py_function = getattr(py_module, function_name)
                    py_function(path)
                    return

        if module in module_by_short_name:
            short_name = module
            module = module_by_short_name[short_name]
            # module is a short name
            # resolve it and add it
            extend_path(path, module, module_by_short_name)
        else:
            # module is a module name from basf2
            path.add_module(module)

    elif callable(module):
        # A convenience function or a module class
        try:
            if issubclass(module, basf2.Module):
                # module is a module class
                # create an instance and add it to the path
                module_instance = module()
                path.add_module(module_instance)
                return
        except TypeError:
            pass

        # module is not a module class
        # try it as a convenience function that add modules to the path
        module(path)

    elif isinstance(module, collections.Iterable):
        # A list of modules or basf2.Path
        modules = module
        for module in modules:
            extend_path(path, module, module_by_short_name)
    else:
        message_template = """
'%s of type %s is neither
* a module instance
* a module (python) class
* a module name
* a add_* function
* a short name resolveable from %s.'
* an iterable of the above (e.g. basf2.Path)
"""
        raise ValueError(message_template % (module,
                                             type(module),
                                             module_by_short_name.keys()))


def get_module_param(module, name):
    parameters = module.available_params()
    for parameter in parameters:
        if name == parameter.name:
            return parameter.values
    else:
        raise AttributeError('%s module does not have a parameter named %s' % (module, name))
