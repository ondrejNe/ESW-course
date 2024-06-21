FROM ubuntu:latest

# Add timezone info and that we are not interactive so that cmake installs without prompts.
ENV TZ=Europe/Prague
ENV DEBIAN_FRONTEND=noninteractive

# Download Linux support tools
RUN apt-get update && \
    apt-get clean &&  \
    apt-get install -y \
        build-essential \
        netcat-openbsd \
        pkg-config \
        valgrind \
        make  \
        wget \
        curl \
        gdb \
        git

# Install dependencies and perf tools
RUN apt-get update && \
    apt-get install -y linux-tools-common linux-tools-generic && \
    apt-get install -y linux-tools-$(uname -r | sed 's/-generic//') || true && \
    apt-get clean

# Download C++ support tools
RUN apt install -y \
        cmake \
        libboost-all-dev

# Download Protobuf support tools
RUN apt install -y \
        protobuf-compiler \
        libprotobuf-dev \
        libprotoc-dev

# Set the dev directory
WORKDIR /home/dev
