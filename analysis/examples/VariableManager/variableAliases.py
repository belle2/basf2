#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Tired of using long names like ``genUpsilon4S(daughter(0, daughter(0, PDG)))``
# (which become even less readable in its ROOT compatible form as demonstrated
# in variableManager.py)?
# Well, despair no longer! With so called "aliases", you can define handy short
# names for your complicated variables.
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org
#

# vm is our shorthand name for the VariableManager instance. The VariableManager
# is responsible for the bookkeeping of our alias definitions.
from variables import variables as vm

# More utilities for managing variables
import variables.utils as vu


# Let's start small and explicitly define alias for some simple variables:
# 'd0_x' is now shorthand for 'daughter(0, x)', etc.
vm.addAlias('d0_x', 'daughter(0, x)')
vm.addAlias('d0_y', 'daughter(0, y)')
vm.addAlias('d0_z', 'daughter(0, z)')

# After each step, you can use vm.printAliases() to print a list of all
# aliases currently defined. In this tutorial we only do it once at the end
# to avoid cluttering the output.

# Now as you see, we have a clear naming convention in our head, but typing it
# all out will quickly become cumbersome if we have lots of variables for lots
# of daughters.
# Let's define similar variables for the momenta, but with just one expression:
vu.create_aliases(
    list_of_variables=["px", "py", "pz"],
    wrapper="daughter(0, {variable})",
    prefix="d0",  # (an underscore is automatically appended to the prefix)
)
# {variable} is replaced by the name of each variable, so we just defined
# the aliases d0_px, d0_py, ...
# Adding more variables to the first daughter is now much easier, because we
# only have to add them to list_of_variables.

# But in fact, this can be done even more conveniently, because the variable
# utils provide us with a dedicated convenience function just to define
# aliases for daughter(.., ..) variables (it's very common after all) Let's
# create aliases for the second daughter

vu.create_daughter_aliases(
    list_of_variables=["x", "y", "z", "px", "py", "pz"],
    indices=1,
)

# In fact, we can also use create_daughter_aliases to create aliases for
# grand(grand(grand))-daughters:
vu.create_daughter_aliases(
    list_of_variables=["PDG"],
    indices=(0, 0, 0),
)
# This will create the alias
# d0_d0_d0_PDG -> daughter(0, daughter(0, daughter(0, PDG)))
# As before, you can also specify a prefix with the prefix keyword argument.

# Another similarly easy convenience function exists for MC truth variables,
# i.e. ``matchedMC(variable)``, which returns the value of the variable for the
# truth particle matched to the particle at hand.
vu.create_mctruth_aliases(
    list_of_variables=["x", "y", "z"],
)
# This creates mc_x, mc_y, etc. You can also customize the prefix with the
# optional prefix keyword argument.

# Let's finally visit one of the more complex convenience functions offered:
# create_aliases_for_selected(...). Let's say you're looking at the decay
# B0 -> [D0 -> pi+ K-] pi0 and want to create variables for the pi^+.
# Of course we could simply do that by hand or by using create_daughter_aliases
# (after all, the pi+ is simply daughter(0, daughter(0, ...))).
# But there's another way, directly from the decay string: Remember that you
# can mark a particle with ``^``: ``B0 -> [D0 -> pi+ K-] pi0``.
# We use that annotation to tell the variable utils which alias we want to
# create:
vu.create_aliases_for_selected(
    list_of_variables=["x", "y", "z"],
    decay_string="B0 -> [D0 -> ^pi+ K-] pi0",
)
# The shorthand name that will be defined by this is by default
# the names of all parent particle names separated by a underscore (if unique).
# If there are more particles with the same name, the daughter indices will
# be appended.
# We can also choose to use a similar naming convention as before:
# d0_d0_... (using daughter indices). For this, add use_names=False to the
# options.
# Take a look at the documentation for create_aliases_for_selected for more
# options and details regarding the naming convention!

# Now the last example is not much shorter than what we did above with
# create_daughter_aliases. But that changes, because we can actually select
# more than one particle:
vu.create_aliases_for_selected(
    list_of_variables=["x", "y", "z"],
    decay_string="B0 -> [^D0 -> pi+ ^K-] ^pi0",
)
# This automatically creates simple aliases for the coordinates in the
# remaining particles in the decay!

# As a final word of warning, keep in mind that aliases are defined globally
# and are not associated with particle lists. Disregarding this often leads to
# clashing aliases and confusion.
# Let's consider a practical example. Say you're considering D0 candidates from
# D0 -> K- pi+ and define
vm.addAlias("K_px", "daughter(0, px)")
# But later (after reconstructing your B meson), you're looking
# at the list of B candidates with B+ -> [D0 -> K- pi+] and define
vm.addAlias("K_px", "daughter(0, daughter(0, px))")
# This last definition will supersede the previous one!
# At least basf2 is nice enough to warn you about this and displays
# "[WARNING] An alias with the name 'K_px' exists and is set to
# 'daughter(0, px)', setting it to 'daughter(0, daughter(0, px))'.
# Be aware: only the last alias defined before processing the events will be
# used!"


# Let's print all of our aliases
vm.printAliases()
