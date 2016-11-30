#
# hunspell minimalistic Dockerfile
# @author Loreto Parisi (loretoparisi at gmail dot com)
# @2016
#

FROM ubuntu:16.04

MAINTAINER Loreto Parisi loretoparisi@gmail.com

# packages list
RUN \
	apt-get update && apt-get install -y \
# build deps
	autoconf \
	automake \
# hunspell deps
	libtool \
	libc6-dev \
	gcc \
	g++ \
	build-essential \
# git
	git

# hunspell
RUN \
	git clone --depth 1 -b v1.5.3 https://github.com/hunspell/hunspell.git && \
	cd hunspell && \
	autoreconf --install && \
	automake --add-missing && \
	./configure && \
	make && \
	make install && \
	ldconfig && \
	make check

# hunspell dict
RUN \	
	git clone --depth 1 -o 7b0f312495f8461b456657e4a3465f82ce2bbd8a https://github.com/wooorm/dictionaries.git && \
# create hunspell dictionaries dir
	mkdir /usr/share/myspell/ && \
	mkdir /usr/share/myspell/dicts && \
# temporary remove specific dictionaries not able to filter
# to be removed as soon as the regex below will be fixed/improved
	rm -rf dictionaries/dictionaries/ca_ES-valencia/ && \
	rm -rf dictionaries/dictionaries/sr_RS-Latn/ && \
	rm -rf dictionaries/dictionaries/ca_ES-valencia/ && \
	rm -rf dictionaries/dictionaries/sr_RS-Latn/ && \
# copy dictionaries
# see http://stackoverflow.com/questions/40878887/regex-in-sed-to-match-a-subpath-in-a-path-with-capturing-groups
	for file in dictionaries/dictionaries/**/*.{dic,aff}; do echo $file | sed 's:.*\([a-z][a-z][_-][A-Z][A-Z]\)/index\(.*\):cp & myDicts/\1\2:' | sh; done
	

# workaround link static lib
# not needed anymore when using `ldconfig`
# @see https://github.com/hunspell/hunspell/issues/435
#RUN \ 
#	cd /usr/lib/x86_64-linux-gnu && \
#	ln -s /usr/local/lib/libhunspell-1.5.so.0 .

# check binaries
RUN \
# check version
	hunspell -v && \
# check dictionaries loaded
	hunspell -D

# set working directory
WORKDIR /root

# defaults command
CMD ["bash"]


