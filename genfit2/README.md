### How to update GenFit in basf2

To update the GenFit version in basf2, we follow the procedure described in https://agira.desy.de/browse/BII-11:

* `git remote add genfit2-on-github "https://github.com/GenFit/GenFit"`
* `git fetch genfit2-on-github`
* `git checkout -b feature/update-genfit`
* `git rm -r genfit2/code2`
* `git read-tree --prefix=genfit2/code2/ -u genfit2-on-github/master`
* `touch genfit2/code2/.imported && git add genfit2/code2/.imported`
* `git commit -m "Update GenFit"`
* `git push --set-upstream origin feature/update-genfit`

or, even better:

* `genfit2-update`

Then, open a pull request and merge it after the review.
