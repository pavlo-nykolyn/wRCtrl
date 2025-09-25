# Author: Pavlo Nykolyn
ARG image_version=
ARG tag_component=${image_version:+:${image_version}}
FROM ubuntu${tag_component}
# configuring the time zone
ENV TZ=Europe/Rome
RUN ln --symbolic --no-dereference --force /usr/share/zoneinfo/${TZ} /etc/localtime && echo ${TZ} > /etc/timezone
# creating the list of repositories that apt can use
RUN echo "deb http://archive.ubuntu.com/ubuntu/ jammy main restricted universe multiverse\ndeb http://archive.ubuntu.com/ubuntu/ jammy-updates main restricted universe multiverse\ndeb http://archive.ubuntu.com/ubuntu/ jammy-security main restricted universe multiverse\ndeb http://archive.ubuntu.com/ubuntu/ jammy-backports main restricted universe multiverse\ndeb http://archive.ubuntu.com/ubuntu/ jammy partner" > /etc/apt/sources.list
# installing the development library for gcc and curl and GNU awk (the latter is needed for some extension of the awk programming language)
RUN apt update && apt install --yes gcc-12 libcurl4-gnutls-dev gawk make
# creating a symbolic link to gcc-12 (I reference the compilation system binary file as gcc within the Makefile)
RUN ln --symbolic --force /bin/gcc-12 /bin/gcc
COPY . /wRCtrl
WORKDIR /wRCtrl
# creating the logs directory that will contain the log data
RUN mkdir logs
# compiling the wRCtrl binary file
RUN make
# environment variables that will hold data necessary to run the relay controller
# the configuration has to follow the format:
# <ip-address>;[<port>];<model>
ENV RELAY_ARRAY_CONFIGURATION=
# a sequence of relay IDs (each one must belong to the interval [1, 8]). The format is
# <id>{ <id>} (for example, "1 2" or "1" or "3 6 8")
ENV IDS=
ENTRYPOINT [ "/bin/bash", "./start_controller.sh" ]
