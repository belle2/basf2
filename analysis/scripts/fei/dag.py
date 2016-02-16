#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Thomas Keck 2015

The DAG (directed acyclic graph) framework automatically resolves dependencies
between given Resources.  A Resource consists of an unique identifier, a
function (or in the simplest case just a value), and (automatic, positional,
keyword) requirements.  All Resources in a DAG object are called in the correct
order, the parameters of the functions are fixed by the given
keyword-requirements. The first parameter is always the Resource object itself.
"""

import hashlib
import basf2
import multiprocessing.pool
import copy
import inspect
import threading
import sys
import pickle
import os
import shutil
import decimal
import collections
import contextlib
import typing

global_lock = threading.RLock()


def create_hash(arguments: typing.Sequence[typing.Any]):
    """
    Creates a unique hash which depends on the given arguments
        @param arguments the hash depends on
    """
    def makeNonAmbiguous(x):
        if isinstance(x, float):
            return decimal.Decimal(x).quantize(decimal.Decimal('.0001'), rounding=decimal.ROUND_DOWN)
        elif isinstance(x, dict):
            items = [(makeNonAmbiguous(k), makeNonAmbiguous(v)) for k, v in sorted(list(x.items()), key=lambda t: t[0])]
            return collections.OrderedDict(items)
        elif isinstance(x, list):
            return list(makeNonAmbiguous(v) for v in x)
        elif isinstance(x, tuple):
            return tuple(makeNonAmbiguous(v) for v in x)
        else:
            return x
    # print(str([str(makeNonAmbiguous(v)) for v in arguments if v is not None]))
    return hashlib.sha1(str([str(makeNonAmbiguous(v)) for v in arguments if v is not None]).encode()).hexdigest()


class Resource(object):
    """
    A Resource is the basic building block of a DAG.
    It consists of an unique identifier, a function and (positional, keyword)
    requirements.  Positional and keyword requirements have to be fullfilled
    before the function of the Resource can be called.  A requirement can be
    fullfilled by the return value of another Resource and is defined by the
    identifier of the Resource.  Each requirement can be either a single
    identifier which corresponds to another Resource or a list of identifiers.
    The keyword requirements are passed to the function as keyword arguments.
    The first argument is always the object itself.

    The function can modify the following members of the Resource object:
        - path (basf2.Path): The path associated with this Resource
        - needed (bool): The path of the Resource is only passed to the final
          path if the Resource is marked as needed or required by a needed
          Resource.
        - cache (bool): The Resource is automatically cached, which is useful if
          the basf2 steering file is run multiple times
        - condition (tuple(string, value)): The path is run in as a conditional path.
        - halt (bool): Resource cannot be provided right now, maybe next turn!

    Additionally the function can read the following members:
        - hash (string): A unique hash of all requirements including positional
          requirements, which are not passed to the function.
        - env (dictionary): The dag environment, which contains steering
          variables, which should not be included in the dependency graph.
    """

    def __init__(self, env: typing.Mapping[str, typing.Any], identifier: str, provider, *args, **kwargs):
        """
        Creates a new Resource
            @param env dag environment dictionary
            @param identifier unique identifier of the Resource
            @param provider source of the Resource, either value or function
            @param args positional requirements
            @param kwargs keyword requirements
        """
        self.identifier = identifier
        self.provider = provider
        self.positional_requirements = args
        self.keyword_requirements = kwargs

        self.automatic_requirements = []
        if isinstance(self.provider, collections.Callable):
            #    try:
            signature = inspect.signature(self.provider)
            for arg in [p.name for p in signature.parameters.values()][1:]:
                if arg not in self.keyword_requirements:
                    self.automatic_requirements.append(arg)
        # except Exception as e:
        #    pass

        # FIXME: compare the objects in a python2 like way: first lists then str
        def key_cmp(x):
            if isinstance(x, list):
                return (0, repr(x))
            else:
                return (1, x)

        self.requires = []
        for v in list(self.positional_requirements) + \
                sorted(list(self.keyword_requirements.values()) +
                       self.automatic_requirements, key=key_cmp):
            if isinstance(v, str):
                self.requires.append(v)
            else:
                self.requires += v

        # Writeable members
        self.needed = False
        self.cache = False
        self.condition = None
        self.halt = False
        self.path = None

        # Readable members
        self.value = None
        self.env = env
        self.hash = None
        self.loaded_from_cache = False

    def __getstate__(self):
        return (self.value, basf2.serialize_path(self.path) if self.path is not
                None else None, self.condition, self.env, self.halt,
                self.needed, self.cache, self.identifier, self.provider,
                self.positional_requirements, self.keyword_requirements,
                self.automatic_requirements, self.requires, self.hash)

    def __setstate__(self, state):
        (self.value, serialized_path, self.condition, self.env, self.halt, self.needed, self.cache,
            self.identifier, self.provider, self.positional_requirements, self.keyword_requirements,
            self.automatic_requirements, self.requires, self.hash) = state
        self.path = basf2.deserialize_path(serialized_path) if serialized_path is not None else None
        self.loaded_from_cache = True

    def __call__(self, arguments: typing.Mapping[str, typing.Any]):
        """
        Calls the underlying function of this actor with the required arguments
            @arguments dictionary of arguments which were required (addtional entries are ignored)
        """
        hash = create_hash([arguments[r] for r in self.requires])
        if self.value is not None and not self.halt and not self.env.get('rerunCachedProviders', False):
            if self.hash == hash:
                if self.env.get('verbose', False):
                    basf2.B2INFO(self.identifier + " provided from cached value " + repr(self.value))
                return self.value
            else:
                basf2.B2WARNING(self.identifier + " cached value has different hash! I won't use it.")
        # Reset hash
        self.hash = hash
        # Reset halt flag
        self.halt = False
        # Reset load from cache
        self.loaded_from_cache = False

        if isinstance(self.provider, collections.Callable):
            parameters = {}
            for key in self.automatic_requirements:
                parameters[key] = arguments[key]
            for (key, value) in self.keyword_requirements.items():
                if isinstance(value, str):
                    parameters[key] = arguments[value]
                else:
                    parameters[key] = [arguments[v] for v in value]

            self.path = basf2.create_path()
            if self.env.get('verbose', False):
                basf2.B2INFO(self.identifier + " called.")
            self.value = self.provider(self, **parameters)
            if self.env.get('verbose', False):
                if self.halt:
                    basf2.B2INFO(self.identifier + " not provided from function, yet.")
                else:
                    basf2.B2INFO(self.identifier + " provided from function")
        else:
            self.value = self.provider
            if self.env.get('verbose', False):
                basf2.B2INFO(self.identifier + " provided from value")

        return self.value

    @contextlib.contextmanager
    def EnableMultiThreading(self):
        global_lock.release()
        yield
        global_lock.acquire()


class DAG(object):
    """
    DAG (directed acyclic graph) used to model dependencies between basf2 reconstruction steps.
    Useful to implement complicated algorithms like FEI or FlavourTagging
    """

    def __init__(self):
        """
        Creates a new DAG object
        """
        #: dictionary containing all Resource objects
        self.resources = collections.OrderedDict()
        #: dictionary containing environment variables
        self.env = {}
        #: All original needed resources
        self.user_flaged_needed = []

    def add(self, identifier: str, provider, *args, **kwargs):
        """
        Adds a new Resource to the DAG
            @param identifier unique identifier of the Resource
            @param provider source of the Resource, either value or function
            @param args positional requirements
            @param kwargs keyword requirements
        """
        self.resources[identifier] = Resource(self.env, identifier, provider, *args, **kwargs)

    def addNeeded(self, identifier: str):
        """
        Mark a resource as needed
            @param identifier of the resource
        """
        self.user_flaged_needed.append(identifier)
        self.resources[identifier].needed = True

    def load_cached_resources(self, cacheFile: str):
        """
        Fills self.resources from given cache file
        """
        if os.path.isfile(cacheFile):
            with open(cacheFile, 'rb') as f:
                cache = pickle.load(f)
                for resource in cache:
                    resource.env = self.env
                    # if 'listCounts' in resource.identifier:
                    #    continue
                    # if 'mcCounts' in resource.identifier:
                    #    continue
                    self.resources[resource.identifier] = resource

    def save_cached_resources(self, cacheFile: str):
        """
        Saves self.resources to cacheFile
        """
        if os.path.isfile(cacheFile):
            backup_file = cacheFile + '.bkp'
            version = 0
            while os.path.isfile(backup_file + str(version)):
                version += 1
            shutil.copyfile(cacheFile, backup_file + str(version))
        with open(cacheFile, 'wb') as f:
            pickle.dump([resource for resource in list(self.resources.values()) if resource.cache], f)

    def showProgress(self, total, done, ready):
        """ Print progress """
        sys.stderr.write("Progress: %g%% (%d/%d actors)\n" % (done * 100.0 / total, done, total))

    def run(self, path):
        """
        Resolve dependencies of the Resources, by extracting step by step the Resources for which
        all their requirements are provided.
        Executes these Resources and store their provided values in a result dictionary.
        @param path basf2 path
        """

        nThreads = self.env.get('nThreads', 1)
        if nThreads > 1:
            p = multiprocessing.pool.ThreadPool(processes=nThreads)

        results = dict()
        results['None'] = None

        chain = []

        resources = list(self.resources.values())
        nResources = len(resources)
        nDone = 0
        while True:
            ready = [x for x in resources if x.identifier not in results and
                     x.identifier not in [y.identifier for y in chain] and
                     all(r in results for r in x.requires)]
            nReady = len(ready)
            if nReady == 0:
                break

            def call_resource(resource, results=results):
                global_lock.acquire()
                results = copy.copy(results)
                resource(results)
                global_lock.release()
                return resource

            if nThreads > 1:
                ready = p.map(call_resource, ready)
            else:
                ready = list(map(call_resource, ready))

            nDone += nReady
            self.showProgress(nResources, nDone, nReady)
            for resource in ready:
                if not resource.halt:
                    results[resource.identifier] = resource.value
            chain += ready

        # Consider all resources to find needed resources
        needed = [x for x in resources if x.needed]
        while len(needed) > 0:
            requirements = []
            for x in needed:
                x.needed = True
                requirements += x.requires
            needed = [x for x in resources if x.identifier in requirements and not x.needed]
        # But finally needed are only resources which are in the chain!
        needed = [x for x in chain if x.needed and x.path is not None and len(x.path.modules()) > 0]
        print("Done")

        if self.env.get('verbose', False):
            print("Needed modules before optimization")
            for n in needed:
                if n.path:
                    for m in n.path.modules():
                        print(m.name())

        # Some modules are certified for parallel processing. Modules which
        # aren't certified should run as late as possible!  Otherwise they slow
        # down all following modules! Therefore we try to move these modules to
        # the end of the needed list.
        needed = self.optimizeForParallelProcessing(needed)

        if self.env.get('verbose', False):
            print("Needed modules after optimization")
            for n in needed:
                if n.path:
                    for m in n.path.modules():
                        print(m.name())

        # The modules in the basf2 path added by the actors are crucial to the
        # FullEventInterpretation Therefore we add the filled path objects of
        # the needed actors to the main basf2 path given as
        # an argument to this function
        for resource in needed:
            if not resource.loaded_from_cache and resource.path is not None and len(resource.path.modules()) > 0:
                if resource.condition is not None:
                    cond_module = basf2.register_module('VariableToReturnValue')
                    cond_module.param('variable', resource.condition[0])
                    cond_module.if_value(resource.condition[1], resource.path, basf2.AfterConditionPath.CONTINUE)
                    cond_module.set_name('VariableToReturnValue(' + resource.condition[0] + ')')
                    path.add_module(cond_module)
                    if self.env.get('verbose', False):
                        for m in resource.path.modules():
                            print("Added conditional module", m.name())
                else:
                    path.add_path(resource.path)
                    if self.env.get('verbose', False):
                        for m in resource.path.modules():
                            print("Added module", m.name())

        if self.env.get('verbose', False):
            self.createDotGraphic(needed)
            self.printMissingDependencies([r for r in resources if r not in chain], list(results.keys()))
            print("Final path")
            for m in path.modules():
                print(m.name())

        return (nDone == nResources) and all([i in results for i in self.user_flaged_needed])

    def optimizeForParallelProcessing(self, needed):
        """
        Reorder the needed modules to optimize for parallel processing without violating the their requirements.
        Essentialy moving all non-parallel-processing certified modules at the end of the list.
        @param needed resources
        """
        def isPPC(resource):
            return all([module.has_properties(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
                        for module in resource.path.modules()])

        pp = list(filter(isPPC, needed))
        if(len(pp) == len(needed)):
            return needed

        while True:
            firstSingleCoreResource = next(x for x in needed if not isPPC(x))
            firstSingleCoreIndex = needed.index(firstSingleCoreResource)

            def canActorRunBeforeFirstSingleCoreResource(resource):
                if not isPPC(resource):
                    return False
                index = needed.index(resource)
                if index <= firstSingleCoreIndex:
                    return True
                provides = [other.identifier for other in needed[firstSingleCoreIndex:index]]
                return not any([r in provides for r in resource.requires])

            optimized = [resource for resource in needed if canActorRunBeforeFirstSingleCoreResource(resource)]
            optimized += [resource for resource in needed if resource not in optimized]
            if optimized == needed:
                break
            needed = optimized

        return optimized

    def printMissingDependencies(self, unresolved, provided):
        """
        Prints the dependencies of all unresolved resources
        @param unresolved unresolved resources
        @param provided provided values
        """
        if len(unresolved) == 0:
            print("All functor dependencies could be fulfilled.")
        else:
            print("The following functors have missing dependencies:")
            for resource in unresolved:
                print(resource.identifier, 'needs', [r for r in resource.requires if r not in provided])

    def createDotGraphic(self, needed):
        """
        Creates a dot graphic showing the dependency graph of all needed actors
        @param needed the needed actors
        """

        print("Saving dependency graph to FEIgraph.dot")
        dotfile = open("FEIgraph.dot", "w")
        dotfile.write("digraph FRdependencies {\n")
        excludeList = [
            'None', 'path', 'hash', 'Geometry', 'Label_', 'Name_',
            'Identifier_', 'MVAConfig_', 'PreCutConfig_', 'PostCutConfig_'
        ]

        for resource in needed:
            if resource.identifier.startswith('SignalProbability') or resource.identifier.startswith('Probability'):
                style = '[shape=box,style=filled,fillcolor=orange]'
            elif resource.identifier.startswith('ParticleList') or resource.identifier.startswith('Particle'):
                style = '[shape=box,style=filled,fillcolor=lightblue]'
            elif resource.identifier.startswith('PreCut') or resource.identifier.startswith('Prefix'):
                style = '[shape=box,style=filled,fillcolor=darkolivegreen1]'
            else:
                style = '[shape=box,style=filled,fillcolor=white]'

            if any(resource.identifier.startswith(exclude) for exclude in excludeList):
                continue

            dotfile.write('"' + resource.identifier + '" ' + style + ';\n')

            for r in set(resource.requires):
                if any(r.startswith(exclude) for exclude in excludeList):
                    continue
                dotfile.write('"' + r + '" -> "' + resource.identifier + '";\n')

        dotfile.write("}\n")
        dotfile.close()
