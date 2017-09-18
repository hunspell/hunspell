# Static Code Checks

This directory contains static code checks. Eventually, they need to move to Makefile.am which is processed by autotools. For now, simply run the shell scripts from this directory and see the results in text and XML format. Each check can also be run directory from Qt Creator as an external tool.


## clang-format

This is a tool to format C/C++/Obj-C code. In all source directories, `hunspell2` and `test`, is a script called `./clang-format.sh` which reformats the source code. Run that before creating a git commit, but also before running the tools mentioned below. This needs installation with `sudo apt-get install clang-format`. For easy of use, there is also a shell script in this directory to run it on all source files at once.


## cloc

This tool counts, and computes differences of, lines of source code and comments. Install with `sudo apt-get install cloc` and run via `./cloc.sh`. The results are in `cloc.txt` and `cloc.xml`. See also https://github.com/AlDanial/cloc


## cppcheck

This is a tool for static C/C++ code analysis. Install with `sudo apt-get install cppcheck`, and run via `/.cppcheck.sh`. The results are in `cppcheck.txt` and `cppcheck.xml`. See also http://cppcheck.sourceforge.net


## vera++

This is a programmable verification and analysis tool for C++. Install with `sudo apt-get install vera++` and run via `./vera++.sh`. The results are in `vera++.txt` and `vera++.xml`. See also https://bitbucket.org/verateam/vera


## Qt Creator

These static code checks and the code reformatter can also be run directly in Qt Creator as external tools. TODO more documentation on this. See also https://sourceforge.net/projects/qtprojecttool
