#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#
# Utility classes, which implement the logic of our functional approach for the FullEventInterpretation.
# We introduce the concept of an actor. An actor is callable and owns a list of requirements and a name.
# A requirement is a unique identifier for a expected value. E.g. Name_B+ is the name of the value B+
# or MVAConfig_e+ is the name of a MVAConfiguration object which is used to train the electron network.
# The call operator of an actor receives the required values and returns a dictionary of provided values.
# The provided values must depend on all the required arguments!
#
# This abstract concept is implemented in this file by the Resource and the Function class.
# All the actors are added to a Sequence object. The Sequence object resolves the dependencies between the actors,
# and runs them in a order which garantuees that all requirements for each actor are fulfilled at runtime.


import hashlib
import basf2


class Resource(object):
    """
    Resources are provide values if certain requirements are fulfilled.
    More specific the Resource class is a generic Functor class, which provides a simple value under a given name.
    It's used to provide things like: Name of a particle, PDG code of a particle, MVA configuration,...
    """
    def __init__(self, name, value, requires=None, strict=True):
        """
        Creates a new Resource
            @param name the name of the resource. Other Actors can require this resource using its name.
            @param value the value of the resource. If another Actor requires this resource its value is passed to the Actor.
            @param requires these requirements have to be fulfilled before calling this Actor
            @param strict if any requirement is None the returned value is None for strict == True
        """
        ##  the name of the resource. Other Actors can require this resource using its name.
        self.name = name
        ## the value of the resource. If another Actor requires this resource its value is passed to the Actor.
        self.value = value
        ## these requirements have to be fulfilled before calling this Actor
        self.requires = [] if requires is None else requires
        ## if any requirement is None the returned value is none for strict == true
        self.strict = strict

    def __call__(self, arguments):
        """
        Returns the given value under the given name
            @arguments dictionary of arguments which were required (addtional entries are ignored)
        """
        arguments = [arguments[r] for r in self.requires if r in arguments]
        if len(arguments) != len(self.requires):
            raise RuntimeError('Requirements are not fulfilled')

        if self.strict and containsNone(arguments):
            return {self.name: None}
        if not self.strict and all([argument is None for argument in arguments]):
            return {self.name: None}
        return {self.name: self.value}


class Function(object):
    """
    This Functor class wraps a normal function into an Actor.
    Every parameter value of the funtion is connected to a unique parameter name, defining the requirements of this actor.
    Imagine you have a function foo:
        def foo(path, particleList): ...
    You can use this class to transform foo to an actor like this:
        Function(foo, path='Path', particleList='K+')
    You can bind multiple requirements to one parameter of the function by passing a list of requirements as the corresponding keyword argument
        Function(foo, path='Path', particleLists=['K+','pi-'])
    """
    def __init__(self, func, **kwargs):
        """
        Creates a new Actor
            @func The function which is called by this Actor
            @kwargs The requirements of the function. The given keyword arguments have to correspond to the parameters of the function!
        """
        ## the name of the actor, at the moment this is simply the name of the function itself
        self.name = func.__name__
        ## the function which is called by this actor
        self.func = func
        ## The kwargs provided by the user.
        self.parameters = kwargs
        ## These requirements have to be fulfilled before calling this Actor
        self.requires = []
        for (key, value) in self.parameters.iteritems():
            if isinstance(value, str):
                self.requires.append(value)
            else:
                self.requires += value

    def __call__(self, arguments):
        """
        Calls the underlying function of this actor with the required arguments
            @arguments dictionary of arguments which were required (addtional entries are ignored)
        """
        arguments = [arguments[r] for r in self.requires if r in arguments]
        if len(arguments) != len(self.requires):
            raise RuntimeError('Requirements are not fulfilled')

        requirement_argument_mapping = dict(zip(self.requires, arguments))
        kwargs = {}
        for (key, value) in self.parameters.iteritems():
            if isinstance(value, str):
                kwargs[key] = requirement_argument_mapping[value]
            else:
                kwargs[key] = [requirement_argument_mapping[v] for v in value]

        result = self.func(**kwargs)
        return result


class Sequence(object):
    """
    The Sequence contains all the actors (Functions and Resources).
    On run, the sequence resolves the dependencies between the actors and calls them in the correct order.
    """

    def __init__(self):
        """ Create a new Sequence """
        ## The Sequence which contains all actors
        self.seq = []

    def addResource(self, *args, **kwargs):
        """ Appends a Resource to the Sequence """
        self.seq.append(Resource(*args, **kwargs))

    def addFunction(self, *args, **kwargs):
        """ Appends a Function to the Sequence """
        self.seq.append(Function(*args, **kwargs))

    def addNeeded(self, key):
        """ Add dummy function which forces the actor which provides key to be needed """
        def fun(a):
            return {}
        self.seq.append(Function(fun, a=key))

    def run(self, path, verbose):
        """
        Resolve dependencies of the Actors, by extracting step by step the actors for which
        all their requirements are provided.
        Executes these Actors and store their provided values in a result dictionary.
        These results are then used to provide the required arguments of the following actors.
        @param path basf2 path
        @param verbose output additional information
        """

        # We loop over all actors and check which actors are ready to run.
        # An actor is ready if all required values are available in the results dictionary.
        # results contains all currently provided value identified by their unique name.
        # All ready actors are called with their required values and the results are updated with the provided return values of the actor.
        # Then all ready actors are added as a list to the chain.
        # If there are no ready actors, the resolution of the dependencies is finished (and maybe incomplete).
        actors = [actor for actor in self.seq]
        results = dict()
        results['Path'] = True
        results['None'] = None
        chain = []
        while True:
            ready = filter(lambda item: all(requirement in results for requirement in item.requires), actors)
            actors = filter(lambda item: not all(requirement in results for requirement in item.requires), actors)
            if len(ready) == 0:
                break
            for actor in ready:
                results['Path'] = actor.path = basf2.create_path()
                actor.provides = actor(results)
                results.update(actor.provides)
            chain.append(ready)

        # Now the chain contains all actors with grantable requirements.
        # The chain is a list of list of actors, all actors in one element of the list depend on the actors in the previous elements.
        # We determine now which of these actors are actually needed. Every actor which doesn't provide a value is defined as needed.
        # Every actor which provides a requirement of a needed actor is also needed, unless the provided value is None.
        # We loop over the chain in reversed direction and add all needed actors to needed.
        # In the end we reverse the needed list, therefore every actor in the lists depends only on the previous actors in the list.
        needed = [actor for level in chain for actor in level if len(actor.provides) == 0]
        for level in reversed(chain):
            needed += list(reversed([actor for actor in level if any((r in actor.provides and actor.provides[r] is not None) for n in needed for r in n.requires)]))
        needed = list(reversed(needed))

        # Some modules are certified for parallel processing. Modules which aren't certified should run as late as possible!
        # Otherwise they slow down all following modules! Therefore we try to move these modules to the end of the needed list.
        needed = self.optimizeForParallelProcessing(needed)

        # The modules in the basf2 path added by the actors are crucial to the FullEventInterpretation
        # Therefore we add the filled path objects of the needed actors to the main basf2 path given as an argument to this function
        for actor in needed:
            if len(actor.path.modules()) > 0:
                path.add_path(actor.path)

        if verbose:
            self.createDotGraphic(needed)
            self.printMissingDependencies(actors, results.keys())

    def optimizeForParallelProcessing(self, needed):
        """
        Reorder the needed modules to optimize for parallel processing without violating the their requirements.
        Essentialy moving all non-parallel-processing certified modules at the end of the list.
        @param needed actors
        """
        def isPPC(actor):
            return all([module.has_properties(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED) for module in actor.path.modules()])

        pp = filter(isPPC, needed)
        if(len(pp) == len(needed)):
            return needed

        while True:
            firstSingleCoreActor = filter(lambda x: not isPPC(x), needed)[0]
            firstSingleCoreIndex = needed.index(firstSingleCoreActor)

            def canActorRunBeforeFirstSingleCoreActor(actor):
                if not isPPC(actor):
                    return False
                index = needed.index(actor)
                if index <= firstSingleCoreIndex:
                    return True
                provides = sum([other.provides.keys() for other in needed[firstSingleCoreIndex:index]], [])
                return not any([r in provides for r in actor.requires])

            optimized = filter(lambda actor: canActorRunBeforeFirstSingleCoreActor(actor), needed)
            optimized += filter(lambda actor: actor not in optimized, needed)
            if optimized == needed:
                break
            needed = optimized

        return optimized

    def printMissingDependencies(self, unresolved, provided):
        """
        Prints the dependencies of all unresolved actors
        @param unresolved unresolved actors
        @param provided provided values
        """
        print "The following functors have missing dependencies:"
        for actor in unresolved:
            print actor.name, 'needs', [r for r in actor.requires if r not in provided]

    def createDotGraphic(self, needed):
        """
        Creates a dot graphic showing the dependency graph of all needed actors
        @param needed the needed actors
        """

        print "Saving dependency graph to FRgraph.dot"
        dotfile = open("FRgraph.dot", "w")
        dotfile.write("digraph FRdependencies {\n")

        for actor in needed:
            for provided in actor.provides:
                style = ''
                if provided.startswith('SignalProbability'):
                    style = '[shape=box,style=filled,fillcolor=orange]'
                elif provided.startswith('ParticleList'):
                    style = '[shape=box,style=filled,fillcolor=lightblue]'
                elif provided.startswith('PreCut'):
                    style = '[shape=box,style=filled,fillcolor=darkolivegreen1]'

                # everything that's special (i.e. listed above)
                if style == '':
                    style = '[shape=box,style=filled,fillcolor=white]'

                if any(provided.startswith(exclude) for exclude in ['Name_', 'MVAConfig_', 'PreCutConfig_']):
                    continue

                dotfile.write('"' + provided + '" ' + style + ';\n')

                for r in set(actor.requires):
                    dotfile.write('"' + r + '" -> "' + provided + '";\n')

        dotfile.write("}\n")
        dotfile.close()


def createHash(*args):
    """
    Creates a unique hash which depends on the given arguments
        @param args arguments the hash depends on
    """
    return hashlib.sha1(str([str(v) for v in args])).hexdigest()


def removeNones(*requirementLists):
    nones = frozenset([index for requirementList in requirementLists for index, object in enumerate(requirementList) if object is None])
    result = [[object for index, object in enumerate(sublist) if index not in nones] for sublist in requirementLists]
    if len(requirementLists) == 1:
        return result[0]
    return result


def getNones(*requirementLists):
    nones = frozenset([index for requirementList in requirementLists for index, object in enumerate(requirementList) if object is None])
    result = [[object for index, object in enumerate(sublist) if index in nones] for sublist in requirementLists]
    if len(requirementLists) == 1:
        return result[0]
    return result


def containsNone(args):
    if isinstance(args, list) or isinstance(args, tuple):
        return any([containsNone(x) for x in args])
    else:
        return args is None
