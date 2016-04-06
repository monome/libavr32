Contributing
==================================

General information about contributing to the monome code ecosystem.

Things you will need
--------------------

 * a [git](https://git-scm.com/) client (used for source version control).
 * a [GitHub](https://github.com/) account (to contribute changes).
 * a C compiler and `make` to build sources.
 * the [libavr32](https://github.com/monome/libavr32) toolchain (if you want to build the firmware).
 * an ssh client (used to authenticate with GitHub).

Getting the code and configuring your environment
-------------------------------------------------

 * Fork the repo of interest into your own GitHub account (more on forking [here](https://help.github.com/articles/fork-a-repo/)).
 * If you haven't configured your machine with an SSH key that's known to github then follow these [directions](https://help.github.com/articles/generating-ssh-keys/).
 * Navigate to a local directory to hold your sources.
 * If you're cloning a repo that has submodules (e.g., the module repos), be sure and clone the repo locally with the `--recursive` flag. 
   * For example, cloning teletype would look like:  `git clone --recursive https://github.com/<my github acct>/teletype.git`.  (Failing to do this will mean you won't get the `libavr32` submodule.)
 * Next add a remote to ensure that you fetch from the master repository, not your clone, when running `git fetch` et al. 
   * For the teletype example again, run `git remote add upstream git@github.com:monome/teletype.git`.

Building and running the code
-----------------------------

Building is generally as simple as:

```
cd src/
make
```

The `README`s for specific modules have any module-specific build instructions. 


Contributing code
-----------------

We gladly accept contributions via GitHub pull requests.

To start working on a patch:

 * `git fetch upstream`
 * `git checkout upstream/master -b name_of_your_branch`
 * Hack away.
 * `git commit -a -m "<your brief but informative commit message>"`
 * `git push origin name_of_your_branch`

To send us a pull request:

 * `git pull-request` (if you are using [Hub](http://github.com/github/hub/)) or
   go to `https://github.com/monome/<specific repo>` and click the
   "Compare & pull request" button.

Once everyone is happy, a maintainer will merge your PR for you.
