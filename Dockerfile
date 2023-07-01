FROM ubuntu:latest

RUN apt-get update -y\
    --no-install-recommends 
RUN apt-get install  -y \
    --no-install-recommends \
        build-essential \
        man-db \  
        valgrind \
        gdb \
        ##automake \ ####
        make \
        ##ca-certificates \ ####
        g++ \
        libtool \ 
        ##pkg-config \ ###
        manpages-dev \
        zip \
        unzip \
        python3 \
        python3-pip \
        git \
        ###openssh-server \ ###
        # dialog \ ###
        llvm \
        clang \ 
        libbsd-dev \
        curl \
        wget \
        zsh \
        nano \
        vim \
        # libreadline-dev \ ###
        moreutils \ 
        ruby \
        gcc \
        gcc-doc \
        htop\
        net-tools
RUN apt-get clean autoclean 
RUN apt-get autoremove --yes
RUN rm -rf /var/lib/{apt,dpkg,cache,log}/

RUN python3 -m pip install --upgrade pip setuptools

WORKDIR /home/
EXPOSE 1234/tcp