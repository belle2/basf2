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
# This abstract concept is implemented in this file by the Property and the Actor class.
# All the actors are added to a Play object. The Play object resolves the dependencies between the actors,
# and runs them in a order which garantuees that all requirements for each actor are fulfilled at runtime.


import hashlib
import basf2
import multiprocessing.pool
import copy
import inspect
import threading
import sys
import cPickle
import os
import shutil

global_lock = threading.RLock()


def create_hash(arguments):
    """
    Creates a unique hash which depends on the given arguments
        @param arguments the hash depends on
    """
    return hashlib.sha1(str([str(v) for v in arguments if v is not None])).hexdigest()


class Property(object):
    """
    Poperties are provide values if certain requirements are fulfilled.
    More specific the Property class is a generic Actor class, which provides a simple value under a given name.
    It's used to provide things like: Name of a particle, PDG code of a particle, MVA configuration,...
    """
    def __init__(self, name, value):
        """
        Creates a new Property
            @param name the name of the resource. Other Actors can require this resource using its name.
            @param value the value of the resource. If another Actor requires this resource its value is passed to the Actor.
        """
        ##  the name of the resource. Other Actors can require this resource using its name.
        self.name = name
        ## the value of the resource. If another Actor requires this resource its value is passed to the Actor.
        self.value = value
        ## there are no requirements
        self.requires = []

    def __call__(self, arguments):
        """
        Returns the given value under the given name
        """
        return {self.name: self.value}


class Collection(object):
    """
    Collections combine a series of identifiers into a single identifier which returns the dict of the identifier and corresponding values
    """
    def __init__(self, name, requires):
        """
        Creates a new Collection
            @param name the name of the collection. Other Actors can require this collection using its name.
            @param requires these requirements have to be fulfilled before calling this Actor
        """
        ##  the name of the resource. Other Actors can require this resource using its name.
        self.name = name
        ## these requirements have to be fulfilled before calling this Actor
        self.requires = [] if requires is None else requires

    def __call__(self, arguments):
        """
        Returns the given value under the given name
            @arguments dictionary of arguments which were required (addtional entries are ignored)
        """
        arguments = {r: arguments[r] for r in self.requires if r in arguments}
        return {self.name: arguments}


class Actor(object):
    """
    This Actor class wraps a normal function into an Actor.
    Every parameter value of the funtion is connected to a unique parameter name, defining the requirements of this actor.
    Imagine you have a function foo:
        def foo(path, particleList): ...
    You can use this class to transform foo to an actor like this:
        Actor(foo, particleList='K+')
    The parameter path is automatically connected to an identifier 'path' whereas particleList ist overwritten to be connected to 'K+'
    You can bind multiple requirements to one parameter of the function by passing a list of requirements as the corresponding keyword argument
        Actor(foo, particleLists=['K+','pi-'])
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
        self.user_parameters = kwargs
        ## Kwargs determined from inspection of cuntion
        self.automatic_parameters = []
        try:
            argspec = inspect.getargspec(func)
            self.automatic_parameters = [arg for arg in argspec.args if arg not in self.user_parameters]
        except Exception as e:
            pass

        ## These requirements have to be fulfilled before calling this Actor
        self.requires = [key for key in self.automatic_parameters]
        for (key, value) in self.user_parameters.iteritems():
            if isinstance(value, str):
                self.requires.append(value)
            else:
                self.requires += value

    def __call__(self, arguments):
        """
        Calls the underlying function of this actor with the required arguments
            @arguments dictionary of arguments which were required (addtional entries are ignored)
        """
        arguments = {r: arguments[r] for r in self.requires if r in arguments}
        kwargs = {key: arguments[key] for key in self.automatic_parameters}
        for (key, value) in self.user_parameters.iteritems():
            if isinstance(value, str):
                kwargs[key] = arguments[value]
            else:
                kwargs[key] = [arguments[v] for v in value]
        result = self.func(**kwargs)
        return result


def call_actor(results, result_cache, path_cache, actor, preload):
    global_lock.acquire()
    results = copy.copy(results)
    results['path'] = actor.path = basf2.create_path()
    results['hash'] = create_hash([results[r] for r in actor.requires])
    key = actor.name + '_' + results['hash']
    if key in result_cache and key in path_cache:
        actor.provides = result_cache[key]
        if not preload:
            actor.path = basf2.deserialize_path(path_cache[key])
    else:
        actor.provides = actor(results)
        if '__cache__' in actor.provides and actor.provides['__cache__']:
            result_cache[key] = actor.provides
            path_cache[key] = basf2.serialize_path(actor.path)
    global_lock.release()


class Play(object):
    """
    The Play contains all the actors (Property, Collection and Actor).
    On run, the play resolves the dependencies between the actors and calls them in the correct order.
    """

    def __init__(self):
        """ Create a new Play """
        ## The sequence which contains all actors
        self.seq = []

    def addProperty(self, *args, **kwargs):
        """ Appends a Property to the Sequence """
        self.seq.append(Property(*args, **kwargs))

    def addCollection(self, *args, **kwargs):
        """ Appends a Property to the Sequence """
        self.seq.append(Collection(*args, **kwargs))

    def addActor(self, *args, **kwargs):
        """ Appends a Actor to the Sequence """
        self.seq.append(Actor(*args, **kwargs))

    def addNeeded(self, key):
        """ Add dummy actor which forces the actor which provides key to be needed """
        def RequireManually(a):
            return {}
        self.seq.append(Actor(RequireManually, a=key))

    def run(self, path, verbose=False, cacheFile=None, preload=False, nProcesses=1):
        """
        Resolve dependencies of the Actors, by extracting step by step the actors for which
        all their requirements are provided.
        Executes these Actors and store their provided values in a result dictionary.
        These results are then used to provide the required arguments of the following actors.
        @param path basf2 path
        @param verbose output additional information
        @param cacheFile file containing a pickled dictionary with hash and result-dict
        @param preload stuff from cache file
        @param nProcesses use n parallel processes for the execution of the actors
        """
        # We loop over all actors and check which actors are ready to run.
        # An actor is ready if all required values are available in the results dictionary.
        # results contains all currently provided value identified by their unique name.
        # All ready actors are called with their required values and the results are updated with the provided return values of the actor.
        # Then all ready actors are added as a list to the chain.
        # If there are no ready actors, the resolution of the dependencies is finished (and maybe incomplete).
        if verbose:
            print "Start execution of Sequence"
        if cacheFile is not None and os.path.isfile(cacheFile):
            result_cache, path_cache = cPickle.load(open(cacheFile, 'r'))
        else:
            result_cache, path_cache = {}, {}
        if nProcesses > 1:
            p = multiprocessing.pool.ThreadPool(processes=nProcesses)
        actors = [actor for actor in self.seq]
        results = dict()
        results['path'] = True
        results['hash'] = ''
        results['None'] = None
        chain = []
        while True:
            if verbose:
                print "Search for actors which are ready for execution"
            ready = filter(lambda item: all(requirement in results for requirement in item.requires), actors)
            actors = filter(lambda item: not all(requirement in results for requirement in item.requires), actors)
            if verbose:
                print "Found {n} actors which are ready for execution".format(n=len(ready))
            if len(ready) == 0:
                break
            if nProcesses > 1:
                def _execute_actor_parallel(actor):
                    call_actor(results, result_cache, path_cache, actor, preload)
                    return actor
                ready = p.map(_execute_actor_parallel, ready)
            else:
                for actor in ready:
                    call_actor(results, result_cache, path_cache, actor, preload)
            if verbose:
                print "Updating result dictionary"
            for actor in ready:
                results.update(actor.provides)
            chain.append(ready)

        if cacheFile is not None:
            if os.path.isfile(cacheFile):
                shutil.copyfile(cacheFile, cacheFile + '.bkp')
            cPickle.dump((result_cache, path_cache), open(cacheFile, 'w'))

        # Now the chain contains all actors with grantable requirements.
        # The chain is a list of list of actors, all actors in one element of the list depend on the actors in the previous elements.
        # We determine now which of these actors are actually needed. Every actor which doesn't provide a value is defined as needed.
        # Every actor which provides a requirement of a needed actor is also needed, unless the provided value is None or the key NotNeeded.
        # We loop over the chain in reversed direction and add all needed actors to needed.
        # In the end we reverse the needed list, therefore every actor in the lists depends only on the previous actors in the list.
        if verbose:
            print "Select needed modules"
        needed = [actor for level in chain for actor in level if len(actor.provides) == 0]
        for level in reversed(chain):
            needed += list(reversed([actor for actor in level if ('__needed__' not in actor.provides or actor.provides['__needed__']) and any((r in actor.provides and actor.provides[r] is not None) for n in needed for r in n.requires)]))
        needed = list(reversed(needed))

        # Some modules are certified for parallel processing. Modules which aren't certified should run as late as possible!
        # Otherwise they slow down all following modules! Therefore we try to move these modules to the end of the needed list.
        if verbose:
            print "Optimize path"
        needed = self.optimizeForParallelProcessing(needed)

        # The modules in the basf2 path added by the actors are crucial to the FullEventInterpretation
        # Therefore we add the filled path objects of the needed actors to the main basf2 path given as an argument to this function
        for actor in needed:
            if len(actor.path.modules()) > 0:
                path.add_path(actor.path)

        if verbose:
            self.createDotGraphic(needed)
            self.printMissingDependencies(actors, results.keys())
        if verbose:
            print "Finished execution of Sequence"

        return len(actors) == 0

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
        if len(unresolved) == 0:
            print "All functor dependencies could be fulfilled."
        else:
            print "The following functors have missing dependencies:"
            for actor in unresolved:
                print actor.name, 'needs', [r for r in actor.requires if r not in provided]

    def createDotGraphic(self, needed):
        """
        Creates a dot graphic showing the dependency graph of all needed actors
        @param needed the needed actors
        """

        print "Saving dependency graph to FEIgraph.dot"
        dotfile = open("FEIgraph.dot", "w")
        dotfile.write("digraph FRdependencies {\n")
        excludeList = ['Path', 'Geometry', 'Label_', 'Name_', 'Identifier_', 'MVAConfig_', 'PreCutConfig_', 'PostCutConfig_']

        for actor in needed:
            for provided in actor.provides:
                if provided.startswith('SignalProbability'):
                    style = '[shape=box,style=filled,fillcolor=orange]'
                elif provided.startswith('ParticleList'):
                    style = '[shape=box,style=filled,fillcolor=lightblue]'
                elif provided.startswith('PreCut'):
                    style = '[shape=box,style=filled,fillcolor=darkolivegreen1]'
                else:
                    style = '[shape=box,style=filled,fillcolor=white]'

                if any(provided.startswith(exclude) for exclude in excludeList):
                    continue

                dotfile.write('"' + provided + '" ' + style + ';\n')

                for r in set(actor.requires):
                    if any(r.startswith(exclude) for exclude in excludeList):
                        continue
                    dotfile.write('"' + r + '" -> "' + provided + '";\n')

        dotfile.write("}\n")
        dotfile.close()


def removeNones(*requirementLists):
    nones = frozenset([index for requirementList in requirementLists for index, object in enumerate(requirementList) if object is None])
    result = [[object for index, object in enumerate(sublist) if index not in nones] for sublist in requirementLists]
    if len(requirementLists) == 1:
        return result[0]
    return result
