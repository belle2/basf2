#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
# Structuring classes, which implement the logic of our functional approach for the FullReconstruction

import hashlib


class Resource(object):
    """
    Resources are provide values if certain requirements are fulfilled.
    To be more specific the Resource class is a generic Functor class, which provides a simple value under a given name.
    It's used to provide things like: Name of a particle, PDG code of a particle, the basf2 path object,...
    """
    def __init__(self, name, x, requires=None):
        """
        Creates a new Resource
            @param name the name of the resource. Other Actors can require this resource using its name.
            @param x the value of the resource. If another Actor requires this resource its value is passed to the Actor.
            @param requires these requirements have to be fulfilled before calling this Actor
        """
        ##  the name of the resource. Other Actors can require this resource using its name.
        self.name = name
        ## the value of the resource. If another Actor requires this resource its value is passed to the Actor.
        self.x = x
        ## these requirements have to be fulfilled before calling this Actor
        self.requires = [] if requires is None else requires

    def __call__(self, *args):
        """
        Returns the given value x under the given name
            @param args additional arguments are ignored
        """
        if containsNone(args):
            return {self.name: None}
        return {self.name: self.x}


class Function(object):
    """
    This Functor class wraps a normal function into an Actor!
    Imagine you have a function foo, which does some crazy stuff:
        def foo(path, particleList): ...
    You can use this class to add foo to the sequence of Actors in the FullReconstruction algorithm like this:
        seq.addFunction(foo, path='Path', particleList='K+')
    The first argument is always the wrapped functions, the following keyword arguments correspond to the arguments of your function,
    the value of the keyword arguments are the names of the requirements the system shall bin to this arguments, when the Functor is called.
    You can bind multiple requirements to one parameter of the function by passing a list of requirements as the corresponding keyword argument
        seq.addFunction(foo, path='Path', particleLists=['K+','pi-'])
    """
    def __init__(self, func, **kwargs):
        """
        Creates a new Actor
            @func The function which is called by this Actor
            @kwargs The requirements of the function. The given keyword arguments have to correspond to the arguments of the function!
        """
        ## the name of the actor, for the moment this is simply the name of the function itself
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

    def __call__(self, *arguments):
        """
        Calls the underlying function of this actor with the required arguments
            @arguments the argument which were required, in the same order as specified in self.requires
        """
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
    The Sequence contains all the Actors (Functions and Resources).
    On run, the sequence solves the dependencies between the Actors
    and calls them in the correct order.
    """

    def __init__(self):
        """ Create a new Sequence """
        ## The Sequence which contains all Functions and Resources
        self.seq = []

    def addResource(self, *args, **kwargs):
        """ Appends a resource to the Sequence """
        self.seq.append(Resource(*args, **kwargs))

    def addFunction(self, *args, **kwargs):
        """ Appends a function to the Sequence """
        self.seq.append(Function(*args, **kwargs))

    def run(self, verbose):
        """
        Resolve dependencies of the Actors, by extracting step by step the Actors for which
        all their requirements are provided.
        Execute these Actors and store their results in results. These results are then used
        to provide the required arguments of the following resources.
        """
        results = dict()
        if verbose:
            print "Saving dependency graph to FRgraph.dot"
            dotfile = open("FRgraph.dot", "w")
            dotfile.write("digraph FRdependencies {\n")

        while len(self.seq) != 0:
            # all items that have their requirements satisfied now
            current = [item for item in self.seq if all(r in results for r in item.requires)]
            # remaining unsatisfied items
            self.seq = [item for item in self.seq if not all(r in results for r in item.requires)]
            for item in current:
                arguments = [results[r] for r in item.requires]
                provides = item(*arguments)
                results.update(provides)
                if verbose:
                    print item.name + ' provides ' + str(provides.keys())
                    for provided in provides.keys():
                        if provided.startswith('Path'):
                            continue
                        style = ''
                        if provided.startswith('SignalProbability'):
                            style = '[shape=box,style=filled,fillcolor=orange]'
                        elif provided.startswith('ParticleList'):
                            style = '[shape=box,style=filled,fillcolor=lightblue]'
                        elif provided.startswith('PreCut'):
                            style = '[shape=box,style=filled,fillcolor=darkolivegreen1]'

                        # everything that's special (i.e. listed above) or is not a Resource gets a box
                        if style != '' or not isinstance(item, Resource):
                            if style == '':
                                style = '[shape=box,style=filled,fillcolor=white]'
                            dotfile.write('"' + provided + '" ' + style + ';\n')

                        for r in set(item.requires):
                            if not r.startswith('Path'):
                                dotfile.write('"' + r + '" -> "' + provided + '";\n')

            if current == []:
                break

        if verbose:
            dotfile.write("}\n")
            dotfile.close()

            print "The following functors have missing dependencies:"
            for x in self.seq:
                print x.name, 'needs', [r for r in x.requires if r not in results]


def createHash(*args):
    """
    Creates a unique has which depends on the given arguments
        @args arguments the hash depends on
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
