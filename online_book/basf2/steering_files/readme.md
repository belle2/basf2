# Steering files

This folder holds steering files for the basf2 introduction.

## If you're a student

Follow the corresponding lessons [here](https://software.belle2.org/development/sphinx/online_book/basf2.html)

## If you're a contributor

You want to improve things? Great!

Keep the following things in mind:

* The steering files `010_...` to `059_...` are building up ONE steering file
* They are included in sphinx, including partial includes specified by line numbers

Therefore:

* If you make changes, make sure to propagate them to all steering files later in
  the series (you can try the `propagate_changes.sh` script)
* Check that the changes between the files are what you expect with e.g.
  `git diff 019_... 029_...`
* If you can, avoid changes that change the line numbers, i.e. avoid insertions
  of new lines or deletions of lines
* If the line numbers do change, go through all lessons on sphinx, look for the
  partial includes and update the line numbers accordingly

Thanks!
