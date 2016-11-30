#
# hunspell minimalistic Dockerfile
# @author Loreto Parisi (loretoparisi at gmail dot com)
# v1.0.0
# @2016 Loreto Parisi (loretoparisi at gmail dot com)
#

FROM ubuntu:16.04

MAINTAINER Loreto Parisi loretoparisi@gmail.com

# working directory
WORKDIR /root

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

# clone hunspell stable tag to let the docker layer to be cached properly
RUN \
	git clone --depth 1 -b v1.5.3 https://github.com/hunspell/hunspell.git

# build hunspell in separate layer in order keep sources anb build cached
RUN \
	cd hunspell && \
	autoreconf --install && \
	automake --add-missing && \
	./configure && \
	make && \
	make install && \
	ldconfig && \
	make check

# clone hunspell dict stable commit
RUN \	
	git clone --depth 1 -o 7b0f312495f8461b456657e4a3465f82ce2bbd8a https://github.com/wooorm/dictionaries.git 

RUN \
# create hunspell dictionaries dir
	bash -c 'mkdir -pv /usr/share/hunspell' && \
	bash -c 'mkdir -pv /tmp/dicts' && \
	ls /usr/share/hunspell && \
# temporary remove specific dictionaries not able to filter
# to be removed as soon as the regex below will be fixed/improved
	cd dictionaries && \
	rm -rf dictionaries/ca_ES-valencia/ && \
	rm -rf dictionaries/sr_RS-Latn/ && \
	rm -rf dictionaries/ca_ES-valencia/ && \
	rm -rf dictionaries/sr_RS-Latn/ && \
# uncomment this to check if you have the dicts
#	bash -c "ls -l dictionaries/*/*.{dic,aff}" && \
# copy dictionaries in hunspell folders
# see http://stackoverflow.com/questions/40878887/regex-in-sed-to-match-a-subpath-in-a-path-with-capturing-groups
	bash -c 'for file in dictionaries/**/*.{dic,aff}; do echo $file | sed "s:.*\([a-z][a-z][_-][A-Z][A-Z]\)/index\(.*\):cp & /tmp/dicts/\1\2:" | sh; done' && \
	bash -c 'cp /tmp/dicts/en_US.dic /tmp/dicts/default.dic' && \
	bash -c 'cp /tmp/dicts/en_US.aff /tmp/dicts/default.aff' && \
	bash -c 'cp /tmp/dicts/* /usr/share/hunspell/' && \
	bash -c 'ls /usr/share/hunspell'
	
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

# defaults command
CMD ["bash"]


