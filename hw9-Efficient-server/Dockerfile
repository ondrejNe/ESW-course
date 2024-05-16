FROM ubuntu:latest

# Add timezone info and that we are not interactive
# so that cmake installs without prompts.
ENV TZ=Europe/Prague
ENV DEBIAN_FRONTEND=noninteractive

# Download Linux support tools
RUN apt-get update && \
    apt-get clean &&  \
    apt-get install -y \
        build-essential \
        wget \
        curl \
        git  \
        python3 \
        cmake

RUN apt install -y \
        pkg-config \
        protobuf-compiler \
        libprotobuf-dev \
        libprotoc-dev \
        libboost-all-dev

# Set the dev directory
WORKDIR /home/dev
