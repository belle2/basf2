#!/usr/bin/env python3

"""
This module contains classes to work with validity intervals. Theres a class
for a single interval, `IntervalOfValidity` and a class to manage a set of
validities, `IoVSet`, which can be used to manipulate iov ranges
"""

import math
from itertools import product


class IntervalOfValidity:
    """Interval of validity class to support set operations like union and intersection"""
    def __init__(self, *iov):
        """Create a new object.

        It can be either instantiated by providing four values or one tuple/list
        with four values for first_exp, first_run, final_exp, final_run
        """
        if len(iov) == 1 and isinstance(iov[0], (list, tuple)):
            iov = iov[0]
        if len(iov) != 4:
            raise ValueError("A iov should have four values")
        #: tuple with the first valid exp, run
        self.__first = tuple(iov[:2])
        #: tuple with the final valid exp, run
        self.__final = tuple(math.inf if x == -1 else x for x in iov[2:])
        if math.isinf(self.__final[0]) and not math.isinf(self.__final[1]):
            raise ValueError(f"Unlimited final experiment but not unlimited run: {self}")
        if self.__first[0] > self.__final[0]:
            raise ValueError(f"First exp larger than final exp: {self}")
        if self.__first[0] == self.__final[0] and self.__first[1] > self.__final[1]:
            raise ValueError(f"First run larger than final run: {self}")
        if self.__first[0] < 0 or self.__first[1] < 0:
            raise ValueError(f"Negative first exp or run: {self}")

    @staticmethod
    def always():
        """Return an iov that is valid everywhere"""
        return IntervalOfValidity(0, 0, -1, -1)

    @property
    def first(self):
        """Return the first valid experiment,run"""
        return self.__first

    @property
    def first_exp(self):
        """Return the first valid experiment"""
        return self.__first[0]

    @property
    def first_run(self):
        """Return the first valid run"""
        return self.__first[1]

    @property
    def final(self):
        """Return the final valid experiment,run"""
        return self.__final

    @property
    def final_exp(self):
        """Return the final valid experiment"""
        return self.__final[0]

    @property
    def final_run(self):
        """Return the final valid run"""
        return self.__final[1]

    def __repr__(self):
        """Return a printable representation"""
        return "{}".format(self.__first + self.__final)

    def __eq__(self, other):
        """Check for equality"""
        if not isinstance(other, IntervalOfValidity):
            return NotImplemented
        return (self.__first, self.__final) == (other.__first, other.__final)

    def __lt__(self, other):
        """Sort by run values"""
        return (self.__first, self.__final) < (other.__first, other.__final)

    def __and__(self, other):
        """Intersection between iovs. Will return None if the payloads don't overlap"""
        return self.intersect(other)

    def __or__(self, other):
        """Union between iovs. Will return None if the iovs don't overlap or connect to each other"""
        return self.union(other, False)

    def __sub__(self, other):
        """Difference between iovs. Will return None if nothing is left over"""
        return self.subtract(other)

    def __hash__(self):
        """Make object hashable"""
        return hash((self.__first, self.__final))

    def subtract(self, other):
        """Return a new iov with the validity of the other removed.
        Will return None if everything is removed.

        Warning:
            If the other iov is in the middle of the validity we will return a tuple of two new iovs

            >>> iov1 = IntervalOfValidity(0,0,10,-1)
            >>> iov2 = IntervalOfValidity(5,0,5,-1)
            >>> iov1 - iov2
            (Iov(0,0,4,inf), IoV(6,0,10,inf))
        """
        if other.first <= self.first and other.final >= self.final:
            # full overlap
            return None
        if other.first > self.first and other.final < self.final:
            # the one we want to remove is in the middle, return a pair of iovs
            # by subtracting two extended once
            end_run = other.first_run - 1
            end_exp = other.first_exp if end_run >= 0 else other.first_exp - 1
            iov1 = self.subtract(IntervalOfValidity(other.first + (-1, -1)))
            iov2 = self.subtract(IntervalOfValidity((0, 0) + other.final))
            return (iov1, iov2)
        if other.first <= self.final and other.final >= self.first:
            # one sided overlap, figure out which side and calculate the remainder
            if self.first < other.first:
                end_run = other.first_run - 1
                end_exp = other.first_exp if end_run >= 0 else other.first_exp - 1
                return IntervalOfValidity(self.first + (end_exp, end_run))
            else:
                start_run = other.final_run + 1
                start_exp = other.final_exp
                if math.isinf(other.final_run):
                    start_exp += 1
                    start_run = 0
                return IntervalOfValidity((start_exp, start_run) + self.final)
        # no overlap so return unchanged
        return self

    def intersect(self, other):
        """Intersection with another iov. Will return None if the payloads don't overlap"""
        if other.first <= self.final and other.final >= self.first:
            return IntervalOfValidity(*(max(self.first, other.first) + min(self.final, other.final)))
        return None

    def union(self, other, allow_startone=False):
        """Union with another iov.

        Warning:
           This method will return None if the iovs don't overlap or connect to
           each other as no union can be formed.

        Parameters:
            other (IntervalOfValidity): IoV to calculate the union with
            allow_startone (bool): If True we will consider run 0 and run 1 the
                first run in an experiment. This means that if one of the iovs has
                un unlimited final run it can be joind with the other iov if the
                experiment number increases and the iov starts at run 0 and 1. If
                this is False just run 0 is considered the next run.

                >>> iov1 = IntervalOfValidity(0,0,0,-1)
                >>> iov2 = IntervalOfValidity(1,1,1,-1)
                >>> iov1.union(iov2, False)
                None
                >>> iov1.union(iov2, True)
                IoV(0,0,1,inf)

        """
        # check the trival case of overlapping
        if other.first <= self.final and other.final >= self.first:
            return IntervalOfValidity(min(self.first, other.first) + max(self.final, other.final))
        # ok, let's do the less simple case where they don't overlap but join directly
        for i1, i2 in (self, other), (other, self):
            if (i1.first == (i2.final_exp, i2.final_run + 1) or
                (math.isinf(i2.final_run) and (i1.first_exp == i2.final_exp + 1) and
                 (i1.first_run == 0 or allow_startone and i1.first_run == 1))):
                return IntervalOfValidity(i2.first + i1.final)
        # no union possible: not directly connected and not overlapping
        return None

    @property
    def tuple(self):
        """Return the iov as a tuple with experiment/run numbers replaced with -1"""
        return self.__first + tuple(-1 if math.isinf(x) else x for x in self.__final)


class IoVSet:
    """A set of iovs.

    This class allows to combine iovs into a set. New iovs can be added with
    `add()` and will be combined with existing iovs if possible.

    The final, minimal number of iovs can be obtained with the `iovs` property

    >>> a = IoVSet()
    >>> a.add((0,0,0,2))
    >>> a.add((0,3,0,5))
    >>> a.add((0,8,0,9))
    >>> a.iovs
    IovSet{IoV(0, 0, 0, 5), IoV(0, 8, 0, 9)}
    """
    def __init__(self, iterable=None, *, allow_overlaps=False, allow_startone=False):
        """Create a new set.

        Parameters:
            allow_overlaps (bool): If False adding which overlaps with any
                existing iov in the set will raise a ValueError.
            allow_startone (bool): If True also join iovs if one covers the
                whole experiment and the next one starts at run 1 in the next
                experiment. Of False they will only be joined if the next one
                starts at run 0.
        """
        #: The set of iovs
        self.__iovs = set()
        #: Whether or not we raise an error on overlaps
        self.__allow_overlaps = allow_overlaps
        #: Whether or not run 1 will be also considered the first run when
        # combining iovs between experiments
        self.__allow_startone = allow_startone
        if iterable is not None:
            for element in iterable:
                self.add(element)

    def add(self, iov):
        """
        Add a new iov to the set.

        The new iov be combined with existing iovs if possible. After the
        operation the set will contain the minimal amount of separate iovs
        possible to represent all added iovs

        >>> a = IoVSet()
        >>> a.add((0,0,0,2))
        >>> a.add((0,3,0,5))
        >>> a.add((0,8,0,9))
        >>> a.iovs
        IovSet{IoV(0, 0, 0, 5), IoV(0, 8, 0, 9)}

        Parameters:
            iov (Union[IoVSet, IntervalOfValidity, tuple(int)]): IoV or
                set of IoVs to add to this set

        Warning:
            This method modifies the set in place
        """
        # we can remove a set from a set :D
        if isinstance(iov, IoVSet):
            for element in iov:
                self.add(element)
            return
        # make sure it's actually an IoV, this will raise an error on failure
        if not isinstance(iov, IntervalOfValidity):
            iov = IntervalOfValidity(iov)
        # and now check for all existing iovs ... (but use a copy since we modify the set)
        for existing in list(self.__iovs):
            # if there's an overlap to the new iov
            if not self.__allow_overlaps and existing & iov:
                raise ValueError(f"Overlap between {existing} and {iov}")
            # and if they can be combined to a bigger iov
            combined = existing.union(iov, self.__allow_startone)
            # if we now have a combined iov, remove the one that we were able to
            # combine it with from the existing iovs because we now check
            # against the combined one. Since the only way to add a new iov is
            # this loop we know all previous existing iovs we checked before
            # didn't have a union with this new iov or any other existing iovs
            # so if the just check the remaining iovs against the new combined
            # one we can cascade combine all iovs in one go.
            if combined is not None:
                self.__iovs.remove(existing)
                iov = combined
        # done, we now have a new iov which combines all existing iovs it had an
        # overlap with and we removed the existing iovs so nothing else to do
        # but add the iov back in the list
        self.__iovs.add(iov)

    def remove(self, iov):
        """Remove and iov or a set of iovs from this set

        After this operation the set will not be valid for the given iov

        >>> a = IoVSet()
        >>> a.add((0,0,10,-1))
        >>> a.remove((1,0,1,-1))
        >>> a.remove((5,0,8,5))
        >>> a.iovs
        IovSet{IoV(0, 0, 0, inf), IoV(2, 0, 4, inf), IoV(8, 6, 10, inf)}

        Parameters:
            iov (Union[IoVSet, IntervalOfValidity, tuple(int)]): IoV or
                set of IoVs to remove from this set

        Warning:
            This method modifies the set in place
        """
        # we can remove a set from a set :D
        if isinstance(iov, IoVSet):
            for element in iov:
                self.remove(element)
            return
        # make sure it's actually an IoV, this will raise an error on failure
        if not isinstance(iov, IntervalOfValidity):
            iov = IntervalOfValidity(iov)
        # and subtract the iov from all existing iovs
        for existing in list(self.__iovs):
            delta = existing - iov
            if delta != existing:
                self.__iovs.remove(existing)
                if isinstance(delta, tuple):
                    # got two new iovs, apparently we split the old one
                    for new in delta:
                        self.__iovs.add(new)
                elif delta is not None:
                    self.__iovs.add(delta)

    def intersect(self, other):
        """Intersect this set with another set and return a new set
        which is valid exactly where both sets have been valid before

        >>> a = IoVSet()
        >>> a.add((0,0,10,-1))
        >>> b = IoVSet()
        >>> b.add((5,0,20,-1))
        >>> a.intersect(b)
        IovSet{IoV(5,0,10,-1))}
        """
        result = IoVSet()
        for a, b in product(self.iovs, other.iovs):
            c = a & b
            if c:
                result.add(c)
        return result

    def contains(self, iov):
        """
        Check if an iov is fully covered by the set

        Parameters:
            iov (Union[IoVSet, IntervalOfValidity, tuple(int)]): IoV or
                set of IoVs to be checked

        Returns:
            True if the full iovs or all the iovs in the given set are fully
            present in this set
        """
        # check if the whole set is in this set: all iovs need to be in here
        if isinstance(iov, IoVSet):
            return all(e in self for e in iov)
        # make sure it's actually an IoV, this will raise an error on failure
        if not isinstance(iov, IntervalOfValidity):
            iov = IntervalOfValidity(iov)
        # and then check all iovs in the set if they cover it
        for existing in self.__iovs:
            if iov - existing is None:
                return True
        return False

    def copy(self):
        """Return a copy of this set"""
        copy = IoVSet(allow_overlaps=self.__allow_overlaps, allow_startone=self.__allow_startone)
        copy.__iovs = set(self.__iovs)
        return copy

    def clear(self):
        """Clear all iovs from this set"""
        self.__iovs = {}

    @property
    def iovs(self):
        """Return the set of valid iovs"""
        return self.__iovs

    def __bool__(self):
        """Return True if the set is not empty"""
        return len(self.__iovs) > 0

    def __contains__(self, iov):
        """Check if an iov is fully covered by the set"""
        return self.contains(iov)

    def __and__(self, other):
        """Return a new set that is the intersection between two sets"""
        return self.intersect(other)

    def __or__(self, other):
        """
        Return a new set that is the combination of two sets: The new set will
        be valid everywhere any of the two sets were valid
        """
        copy = self.copy()
        copy.add(other)
        return copy

    def __sub__(self, other):
        """Return a new set which is only valid for where a is valid but not b"""
        copy = self.copy()
        copy.remove(other)
        return copy

    def __iter__(self):
        """Loop over the set of iovs"""
        return iter(self.__iovs)

    def __repr__(self):
        """Return a printable representation"""
        return "IoVSet" + repr(self.__iovs)
