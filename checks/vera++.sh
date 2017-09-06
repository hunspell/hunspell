#TODO see also comments in cppcheck.sh

#TODO need to enable and fine-tune all rules
#TODO need to disable tab reporting
#TODO but need first local profile vera++.profile, see https://bitbucket.org/verateam/vera/issues/60/allow-to-use-absolute-path-for-profile

cd ..

# omitting config.h

ls \
src/hunspell2/*.hxx src/hunspell2/*.cxx \
src/parsers/*.hxx src/parsers/*.cxx \
src/tools/*.cxx \
| grep -v hunspell.cxx \
| vera++ \
-p vera++.profile \
-s \
-o checks/vera++.txt

ls \
src/hunspell2/*.hxx src/hunspell2/*.cxx \
src/parsers/*.hxx src/parsers/*.cxx \
src/tools/*.cxx \
| grep -v hunspell.cxx \
| vera++ \
-p vera++.profile \
-P max-line-length=80 \
-s \
-x checks/vera++.xml

cd checks
