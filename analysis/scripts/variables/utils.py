#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import functools
import collections
import re
from variables import variables as _variablemanager
from variables import std_vector as _std_vector
from typing import Iterable, Union, List, Tuple, Optional


def create_aliases(list_of_variables: Iterable[str], wrapper: str, prefix: str) -> List[str]:
    """
    The function creates aliases for variables from the variables list with given wrapper
    and returns list of the aliases.

    If the variables in the list have arguments (like ``useLabFrame(p)``) all
    non-alphanumeric characters in the variable will be replaced by underscores
    (for example ``useLabFrame_x``) for the alias name.

    >>> list_of_variables = ['M','p','matchedMC(useLabFrame(px))']
    >>> wrapper = 'daughter(1,{variable})'
    >>> prefix = 'pref'
    >>> print(create_aliases(list_of_variables, wrapper, prefix))
    ['pref_M', 'pref_p', 'pref_matchedMC_useLabFrame_px']
    >>> from variables import variables
    >>> variables.printAliases()
    [INFO] =====================================
    [INFO] The following aliases are registered:
    [INFO] pref_M                        --> daughter(1,M)
    [INFO] pref_matchedMC_useLabFrame_px --> daughter(1,matchedMC(useLabFrame(px)))
    [INFO] pref_p                        --> daughter(1,p)
    [INFO] =====================================

    Parameters:
        list_of_variables (list(str)): list of variable names
        wrapper (str): metafunction taking variables from list_of_variables as a parameter \
        (``<metafunction>(<some configs>, {variable} ,<some other configs>)``
        prefix (str): alias prefix used for wrapped variables.

    Returns:
        list(str): new variables list
    """
    replacement = re.compile('[^a-zA-Z0-9]+')
    aliases = []
    for var in list_of_variables:
        # replace all non-safe characters for alias name with _ (but remove from the end)
        safe = replacement.sub("_", var).strip("_")
        aliases.append(f"{prefix}_{safe}")
        _variablemanager.addAlias(aliases[-1], wrapper.format(variable=var))

    return aliases


def get_hierarchy_of_decay(decay_string: str) -> List[List[Tuple[int, str]]]:
    """
    This function returns paths of the particles selected in decay string. For
    each selected particle return a list of (index, name) tuples which indicate
    which daughter index to choose to arrive at the selected particle.

    For example for the decay string ``B+ -> [ D+ -> pi0 ^K+ ] pi0`` the
    resulting path for the K+ would be ``[(0, 'D'), (1, 'K')]``: The K is the
    second daughter of the first daughter of the B+

    >>> get_hierarchy_of_decay('B+ -> [ D+ -> ^K+ pi0 ] pi0')
    [[(0, 'D'), (0, 'K')]]

    Every selected particle has its own path so if multiple particles are
    collected a list of paths is returned

    >>> get_hierarchy_of_decay('B+ -> [ D+ -> ^K+ pi0 ] ^pi0')
    [[(0, 'D'), (0, 'K')], [(1, 'pi0')]]

    If the mother particle is selected an empty list will be returned as its path

    >>> get_hierarchy_of_decay('^B+ -> ^pi+ pi-')
    [[], [(0, 'pi')]

    Parameters:
        decay_string (str): Decay string with selected particles

    Returns:
        list(list(tuple(int, str))): list of hierarchies of selected particles.
    """
    from ROOT import Belle2
    d = Belle2.DecayDescriptor()
    if not d.init(decay_string):
        raise ValueError("Invalid decay string")

    selected_particles = []
    for path in d.getHierarchyOfSelected():
        selected_particles.append([tuple(e) for e in path[1:]])
    return selected_particles


def create_daughter_aliases(
        list_of_variables: Iterable[str],
        indices: Union[int, Iterable[int]],
        prefix="", include_indices=True
) -> List[str]:
    """Create Aliases for all variables for a given daughter hierarchy

    Arguments:
        list_of_variables (list(str)): list of variables to create aliases for
        indices (int or list(int)): index of the daughter, grand-daughter, grand-grand-daughter,
            and so forth
        prefix (str): optional prefix to prepend to the aliases
        include_indices(bool): if set to True (default) the aliases will contain
            the daughter indices as dX_dY_dZ...

    Returns:
        list(str): new variables list

    * create aliases for the second daughter as "d1_E", "d1_M" (daughters start at 0)

      >>> create_daughter_aliases(["E", "m"], 1)
      ['d1_E', 'd1_m']
      >>> from variables import variables
      >>> variables.printAliases()
      [INFO] =========================
      [INFO] Following aliases exists:
      [INFO] 'd1_E' --> 'daughter(1,E)'
      [INFO] 'd1_m' --> 'daughter(1,m)'
      [INFO] =========================


    * create aliases for the first grand daughter of the second daughter,
      starting with "my" and without including the indices, resulting in "my_E", "my_m"

      >>> create_daughter_aliases(["E", "m"], [1, 0], prefix="my", include_indices=False)
      ['my_E', 'my_m']
      >>> from variables import variables
      >>> variables.printAliases()
      [INFO] =========================
      [INFO] Following aliases exists:
      [INFO] 'my_E' --> 'daughter(1,daughter(0,E))'
      [INFO] 'my_m' --> 'daughter(1,daughter(0,m))'
      [INFO] =========================

    * create aliases for the second grand grand daughter of the third grand
      daughter of the fifth daugther, starting with my and including the
      indices, resulting in "my_d4_d2_d1_E", "my_d4_d2_d1_m"

      >>> create_daughter_aliases(["E", "m"], [4, 2, 1], prefix="my")
      ['my_d4_d2_d1_E', 'my_d4_d2_d1_m']
      >>> from variables import variables
      >>> variables.printAliases()
      [INFO] =========================
      [INFO] Following aliases exists:
      [INFO] 'my_d4_d2_d1_E' --> 'daughter(4,daughter(2,daughter(1,E))'
      [INFO] 'my_d4_d2_d1_m' --> 'daughter(4,daughter(2,daughter(1,m))'
      [INFO] =========================

    """

    if not isinstance(indices, collections.Iterable):
        indices = [indices]

    if include_indices:
        prefix = functools.reduce(lambda x, y: f"{x}_d{y}", indices, prefix).lstrip("_")

    template = functools.reduce(lambda x, y: f"daughter({y},{x})", reversed(indices), "{variable}")
    return create_aliases(list_of_variables, template, prefix)


class DecayParticleNode:
    """
    Class to present selected particles from a DecayString as tree structure.
    For each node of the tree we safe the name of the particle, whether it is
    selected and a dictionary of all children (as mapping decayIndex -> Node)
    """

    def __init__(self, name):
        """Just set default values"""
        #: name of the particle
        self.name = name
        #: whether or not this particle is selected
        self.selected = False
        #: mapping of children decayIndex->Node
        self.children = {}

    def get_prefixes(self, always_include_indices=False, use_relative_indices=False):
        """
        Recursively walk through the tree of selected particles and return a list
        of prefixes for aliases and a tuple of decay indexes for that prefix.

        For example for ``B0 -> [D0 -> ^pi+] ^pi0`` it might return

        >>> DecayParticleNode.build('^B0 -> [D0 -> ^pi+] ^pi0').get_prefixes()
        [ ("", None), ("D0_pi", (0, 0)), ("pi0", (1,)) ]

        and to create aliases from these one would use the indices as arguments for
        the b2:var:`daughter` meta variable.

        This function will make sure that prefix names are unique: If there are
        multiple siblings of one node with the same particle name they will be
        distinguised by either suffixing them with the decay index (if
        ``use_relative_indices=False``) or they will just be enumerated
        starting at 0 otherwise.

        Arguments:
            always_include_indices (bool): If True always add the index of the
                particle to the prefix, otherwise the index is only added if
                more than one sibling of the same particle exist.
            use_relative_indices (bool): If True the indices used will **not**
                be the daughter indices in the full decay string but just the
                relative indices: If multiple sibling particles with the same
                name they will be just numbered starting at zero as they appear
                in the aliases.
        """
        return self.__walk(always_include_indices, use_relative_indices, "", tuple())

    def __walk(self, always_include_indices, use_relative_indices, current_prefix, current_path):
        """Recursively walk the tree and collect all prefixes

        See:
            `get_prefixes`

        Arguments:
            always_include_indices (bool): see `get_prefixes()`
            use_relative_indices (bool): see `get_prefixes()`
            current_prefix: the current prefix so far collected from any parent
                particle.
            current_path: the current path of indices so far collected from any
                parent particle.
        """

        result = []
        # are we the mother particle and selected selected? if so, add a "no-prefix" to the output
        if not current_path and self.selected:
            result.append(("", None))

        # count the particle names of all daughters so that we know which ones we
        # have to index
        names = collections.Counter(e.name for e in self.children.values())
        # if we use relative indices start counting them at zero
        relative_indices = collections.defaultdict(int)

        # now loop over all children
        for index, c in sorted(self.children.items()):
            # prepare the full index path
            full_path = current_path + (index,)
            # and prepare the prefix
            prefix = current_prefix + c.name
            # is this particle name ambiguous or are all indices requested? add index
            if always_include_indices or names[c.name] > 1:
                prefix += "_{}".format(relative_indices[c.name] if use_relative_indices else index)
                # always increase the relative indices
                relative_indices[c.name] += 1

            # if the particle is selected add the prefix and the path
            if c.selected:
                result.append((prefix, full_path))

            # but in any case also process all children recursively
            result += c.__walk(always_include_indices, use_relative_indices, prefix + "_", full_path)

        # done, return all prefixes and their paths
        return result

    @classmethod
    def build(cls, decay_string):
        """Build a tree of selected particles from a `DecayString`

        This will return a `DecayParticleNode` instance which is the top of a
        tree of all the selected particles from the decat string.

        Arguments:
            decay_string (str): `DecayString` containing at least one selected particle
        """
        selected = get_hierarchy_of_decay(decay_string)
        if not selected:
            raise ValueError("No particle selected in decay string")
        # create the top of the tree
        top = cls("")
        # now loop over all selected particles
        for path in selected:
            current = top
            # and walk through the path
            for index, name in path:
                # creating tree children as needed
                if index not in current.children:
                    current.children[index] = cls(name)
                # and update the pointer
                current = current.children[index]
            # after walking the tree the pointer is at the selected particle so
            # just set the selected to True
            current.selected = True

        # done, return the tree
        return top


def create_aliases_for_selected(
        list_of_variables: List[str],
        decay_string: str,
        prefix: Optional[Union[str, List[str]]] = None,
        *,
        use_names=True,
        always_include_indices=False,
        use_relative_indices=False
) -> List[str]:
    """
    The function creates list of aliases for given variables so that they are calculated for
    particles selected in decay string. That is for each particle selected in
    the decay string an alias is created to calculate each variable in the
    ``list_of_variables``.

    If ``use_names=True`` (the default) then the names of the aliases are assigned as follows:

    * If names are unambiguous, it's semi-laconic :doc:`DecayString` style: The
      aliases will be prefixed with the names of all parent particle names
      separated by underscore. For example given the decay string ``B0 -> [D0 -> ^pi+ K-] pi0``
      the aliases for the ``pi+` will start with ``D0_pi_`` followed by the
      variable name.

      >>> list_of_variables = ['M','p']
      >>> decay_string = 'B0 -> [D0 -> ^pi+ K-] pi0'
      >>> create_aliases_for_selected(list_of_variables, decay_string)
      ['D0_pi_M', 'D0_pi_p']
      >>> from variables import variables
      >>> variables.printAliases()
      [INFO] =========================
      [INFO] Following aliases exists:
      [INFO] 'D0_pi_M' --> 'daughter(0,daughter(0,M))'
      [INFO] 'D0_pi_p' --> 'daughter(0,daughter(0,p))'
      [INFO] =========================


    * If names are ambiguous because there are multiple daughters with the same
      name these particles will be followed by their daughter index. For example
      given the decay string ``B0 -> [D0 -> ^pi+:1 ^pi-:2 ^pi0:3 ] ^pi0:4``
      will create aliases with the following prefixes for particle with the
      corresponding number as list name:

      1. ``D0_pi_0_``
      2. ``D0_pi_1_``
      3. ``D0_pi0_``
      4. ``pi0_``

      >>> list_of_variables = ['M','p']
      >>> decay_string = 'B0 -> [D0 -> ^pi+ ^pi- ^pi0] ^pi0'
      >>> create_aliases_for_selected(list_of_variables, decay_string)
      ['D0_pi_0_M', 'D0_pi_0_p', 'D0_pi_1_M', 'D0_pi_1_p',
      'D0_pi0_M', 'D0_pi0_p', 'pi0_M', 'pi0_p']
      >>> from variables import variables
      >>> variables.printAliases()
      [INFO] =========================
      [INFO] Following aliases exists:
      [INFO] 'D0_pi0_M' --> 'daughter(0,daughter(2,M))'
      [INFO] 'D0_pi0_p' --> 'daughter(0,daughter(2,p))'
      [INFO] 'D0_pi_0_M' --> 'daughter(0,daughter(0,M))'
      [INFO] 'D0_pi_0_p' --> 'daughter(0,daughter(0,p))'
      [INFO] 'D0_pi_1_M' --> 'daughter(0,daughter(1,M))'
      [INFO] 'D0_pi_1_p' --> 'daughter(0,daughter(1,p))'
      [INFO] 'D0_pi_M' --> 'daughter(0,daughter(0,M))'
      [INFO] 'D0_pi_p' --> 'daughter(0,daughter(0,p))'
      [INFO] 'pi0_M' --> 'daughter(1,M)'
      [INFO] 'pi0_p' --> 'daughter(1,p)'
      [INFO] =========================

    * The user can select to always include the index even for unambiguous
      particles by passing ``always_include_indices=True``

    * The user can choose two different numbering schemes: If
      ``use_relative_indices=False`` the original decay string indices will be
      used if a index is added to a particle name.

      But if ``use_relative_indices=True`` the indices will just start at zero for each
      particle which is part of the prefixes. For example for ``B0-> e+ ^e-``

      >>> create_aliases_for_selected(['M'], 'B0-> mu+ e- ^e+ ^e-', use_relative_indices=False)
      ['e_2_M', 'e_3_M']
      >>> create_aliases_for_selected(['M'], 'B0-> mu+ e- ^e+ ^e-', use_relative_indices=True)
      ['e_0_M', 'e_1_M']

    If ``use_names=False`` the aliases will just start with the daughter indices
    of all parent particles prefixed with a ``d`` and separated by underscore. So
    for the previous example ``B0 -> [D0 -> ^pi+:1 ^pi-:2 ^pi0:3 ] ^pi0:4``
    this would result in aliases starting with

    1. ``d0_d0_``
    2. ``d0_d1_``
    3. ``d0_d2_``
    4. ``d1_``

    In this case the ``always_include_indices`` and ``use_relative_indices``
    arguments are ignored.

    The naming can be modified by providing a custom prefix for each selected
    particle. In this case the parameter ``prefix`` needs to be either a simple
    string if only one particle is selected or a list of strings with one
    prefix for each selected particle.

    >>> list_of_variables = ['M','p']
    >>> decay_string = 'B0 -> [D0 -> ^pi+ ^pi- pi0] pi0'
    >>> create_aliases_for_selected(list_of_variables, decay_string, prefix=['pip', 'pim'])
    ['pip_M', 'pip_p', 'pim_M', 'pim_p']
    >>> from variables import variables
    >>> variables.printAliases()
    [INFO] =========================
    [INFO] Following aliases exists:
    [INFO] 'pim_M' --> 'daughter(0,daughter(1,M))'
    [INFO] 'pim_p' --> 'daughter(0,daughter(1,p))'
    [INFO] 'pip_M' --> 'daughter(0,daughter(0,M))'
    [INFO] 'pip_p' --> 'daughter(0,daughter(0,p))'
    [INFO] =========================

    If the mother particle itself is selected the input list of variables will
    also be added to the returned list of created aliases. If custom prefixes
    are supplied then aliases will be created for the mother particle as well:

    >>> create_aliases_for_selected(['M', 'p'], '^B0 -> pi+ ^pi-')
    ['M', 'p', 'pi_M', 'pi_p']
    >>> create_aliases_for_selected(['M', 'p'], '^B0 -> pi+ ^pi-', prefix=['MyB', 'MyPi'])
    ['MyB_M', 'MyB_p', 'MyPi_M', 'MyPi_p']

    Parameters:
        list_of_variables (list(str)): list of variable names
        decay_string (str): Decay string with selected particles
        prefix (str, list(str)): Custom prefix for all selected particles
        use_names (bool): Include the names of the particles in the aliases
        always_include_indices (bool): If ``use_names=True`` include the decay
            index of the particles in the alias name even if the particle could
            be uniquely identified without them.
        use_relative_indices (bool): If ``use_names=True`` use a relative
            indicing which always starts at 0 for each particle appearing in
            the alias names independent of the absolute position in the decay
            string

    Returns:
        list(str): new variables list
    """

    selected_particles = DecayParticleNode.build(decay_string)
    prefixes = selected_particles.get_prefixes(always_include_indices, use_relative_indices)
    # transpose -> convert [(prefix, path), (prefix, path) ...] into
    # (prefix, prefix...), (path, path, ...)
    prefixes, paths = zip(*prefixes)

    alias_list = []
    # in most cases we don't want to add daughter indices `dM_dN` to the
    # aliases
    include_indices = False
    # but we might have custom prefixes
    if prefix is not None:
        # check custom prefix to be a list
        if isinstance(prefix, str):
            prefix = [prefix]
        # and make sure we have the correct amount
        if len(prefix) != len(prefixes):
            raise ValueError("Number of selected particles does not match number of supplied custom prefixes")
        # final check: make sure we don't have duplicate prefixes in here
        prefix_counts = collections.Counter(prefix)
        if max(prefix_counts.values()) > 1:
            raise ValueError("Prefixes need to be unique")
        # ok, just override calculated prefixes
        prefixes = prefix
    elif not use_names:
        # otherwise, if we don't use names we just override the prefixes
        # containing the names to be empty and make sure we include the
        # daughter indices
        prefixes = [""] * len(prefixes)
        include_indices = True

    for prefix, path in zip(prefixes, paths):
        if path is None:
            # mother particle selected, just create custom aliases which
            # don't do anything special if the prefix is not empty
            if prefix:
                alias_list += create_aliases(list_of_variables, "{variable}", prefix)
            else:
                # but if prefix is empty just return the variables
                alias_list += list_of_variables
        else:
            # mother particle selected, just create custom aliases
            alias_list += create_daughter_aliases(list_of_variables, path, prefix, include_indices)

    return alias_list


def create_mctruth_aliases(
        list_of_variables: Iterable[str],
        prefix="mc"
) -> List[str]:
    """
    The function wraps variables from the list with 'matchedMC()'.

    >>> list_of_variables = ['M','p']
    >>> create_mctruth_aliases(list_of_variables)
    ['mc_M', 'mc_p']
    >>> from variables import variables
    >>> variables.printAliases()
    [INFO] =========================
    [INFO] Following aliases exists:
    [INFO] 'mc_M' --> 'matchedMC(M)'
    [INFO] 'mc_p' --> 'matchedMC(p)'
    [INFO] =========================


    Parameters:
        list_of_variables (list(str)): list of variable names

    Returns:
        list(str): list of created aliases
    """
    return create_aliases(list_of_variables, 'matchedMC({variable})', prefix)


def add_collection(list_of_variables: Iterable[str], collection_name: str) -> str:
    """
    The function creates variable collection from the given list of variables
    It wraps the `VariableManager.addCollection` method which is not particularly user-friendly.

    Example:

        Defining the collection
        >>> variables.utils.add_collection(['p','E'], "my_collection")

        Passing it as an argument to variablesToNtuple
        >>> modularAnalysis.variablesToNtuple(variables=['my_collection'], ...)

    Parameters:
        list_of_variables (list(str)): list of variable names
        collection_name (str): name of the collection

    Returns:
        str: name of the variable collection
    """

    _variablemanager.addCollection(collection_name, _std_vector(*tuple(list_of_variables)))
    return collection_name


def create_isSignal_alias(aliasName, flags):
    """
    Make a `VariableManager` alias for a customized :b2:var:`isSignal`, which accepts specified mc match errors.

    .. seealso:: see :doc:`MCMatching` for a definition of the mc match error flags.

    The following code defines a new variable ``isSignalAcceptMissingGammaAndMissingNeutrino``, which is same
    as :b2:var:`isSignal`, but also accepts missing gamma and missing neutrino

    >>> create_isSignal_alias("isSignalAcceptMissingGammaAndMissingNeutrino", [16, 8])

    Logically, this
    ``isSignalAcceptMissingGammaAndMissingNeutrino`` =
    :b2:var:`isSignalAcceptMissingGamma` || :b2:var:`isSignalAcceptMissingNeutrino`.

    In the example above, create_isSignal_alias() creates ``isSignalAcceptMissingGammaAndMissingNeutrino`` by
    unmasking (setting bits to zero)
    the ``c_MissGamma`` bit (16 or 0b00010000) and ``c_MissNeutrino`` bit (8 or 0b00001000) in mcErrors.

    For more information, please check this `example script <https://stash.desy.de/projects/B2/repos/basf2/
    browse/analysis/examples/VariableManager/isSignalAcceptFlags.py>`_.

    Parameters:
        aliasName (str): the name of the alias to be set
        flags (list(int)): a list of the bits to unmask
    """

    mask = 0
    for flag in flags:
        if isinstance(flag, int):
            mask |= flag
        else:
            informationString = "The type of input flags of create_isSignal_alias() should be integer."
            informationString += "Now one of the input flags is " + str(int) + " ."
            raise ValueError(informationString)

    _variablemanager.addAlias(aliasName, "passesCut(unmask(mcErrors, %d) == %d)" % (mask, 0))
