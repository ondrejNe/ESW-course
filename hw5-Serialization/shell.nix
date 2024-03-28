let
  sources = import ./npins;
  pkgs = import sources.nixpkgs {};
in
with pkgs;
mkShell {
  packages = [
    pkg-config
    meson
    ninja
    boost
    jsoncpp
    jdk17
    maven
    protobuf
    cmake
    avro-cpp
  ];

  shellHook = ''
      export PATH=$PATH:/nix/store/0zihsc1dzpbdzidm052zl765hg3rc3hz-avro-c++-1.11.0/bin
      export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nix/store/0zihsc1dzpbdzidm052zl765hg3rc3hz-avro-c++-1.11.0/lib
      echo "Added avro-c++ to PATH and LD_LIBRARY_PATH"
    '';
}
