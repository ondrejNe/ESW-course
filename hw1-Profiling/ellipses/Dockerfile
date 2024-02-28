# Use a base image with Nix installed
FROM nixos/nix

# Add timezone info and that we are not interactive
# so that cmake installs without prompts.
ENV TZ=Europe/Prague
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /home/dev
