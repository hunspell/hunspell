#TODO see also comments in cppcheck.sh

#TODO need to enable and fine-tune all rules
#TODO need to disable tab reporting
#TODO but need first local profile vera++.profile, see https://bitbucket.org/verateam/vera/issues/60/allow-to-use-absolute-path-for-profile

cd ..

# omitting config.h

vera++ \
-s -p vera++.profile -P max-line-length=80 src/hunspell2/*xx tests/*xx \
-o checks/vera++.txt

vera++ \
-s -p vera++.profile -P max-line-length=80 src/hunspell2/*xx tests/*xx \
-x checks/vera++.xml

cd checks
