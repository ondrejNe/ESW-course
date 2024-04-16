# Use a base image with Nix installed
FROM nixos/nix:latest

# Add timezone info and that we are not interactive
# so that cmake installs without prompts.
ENV TZ=Europe/Prague
ENV DEBIAN_FRONTEND=noninteractive

COPY . /home/dev
WORKDIR /home/dev

# Deps
#RUN sudo install -d -m755 -o $(id -u) -g $(id -g) /nix
#RUN curl -L https://nixos.org/nix/install | sh
#RUN source $HOME/.nix-profile/etc/profile.d/nix.sh

# C++ build
#RUN nix-shell --run 'meson setup builddir'
#RUN nix-shell --run 'meson compile -C builddir'

# Java build
#RUN nix-shell --run 'mvn compile'
