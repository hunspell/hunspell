#TODO Set up local profile vera++.profile with:
#    sudo ln -s /home/.../workspace/hunspell/checks/vera++.profile \
#    /usr/lib/vera++/profiles/vera__.profile
#See also https://bitbucket.org/verateam/vera/issues/60/allow-to-use-absolute-path-for-profile

cd ..

# Header file config.h is omitted.

vera++ \
-s -p vera++.profile -P max-line-length=80 src/hunspell2/*.[ch]xx tests/*.[ch]xx \
-o checks/vera++.txt

#vera++ \
#-s -p vera++.profile -P max-line-length=80 src/hunspell2/*.[ch]xx tests/*.[ch]xx \
#-x checks/vera++.xml

cd checks
