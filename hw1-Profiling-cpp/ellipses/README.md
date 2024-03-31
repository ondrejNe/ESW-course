To build the project, install dependencies or use Nix shell:

    nix-shell

This may take long time as OpenCV needs to be recompiled, because we
use a non-default configuration with GTK3 support. To skip the
compilation, you can use our Nix binary cache provided by
[cachix.org][]. Just install the `cachix` tool and enable the [esw
cache][]:

    nix-env -iA cachix -f https://cachix.org/api/v1/install
    cachix use esw
    nix-shell

Then build the project with Meson:

    meson setup build --buildtype=plain
    meson compile --verbose -C build

Using `--buildtype=plain` causes Meson not to use any debug and
optimization flags when calling the compiler. These are manually set
in `meson.build` for this task. Investigate how compiler options can
be tuned.

Using `--verbose` prints commands used to build the project to the
terminal. Investigate that output.

And finally run the project:

    ./build/ellipse/find_ellipse -h
    ./build/ellipse/find_ellipse images/table.jpg 1000 no-gui

[cachix.org]: https://www.cachix.org/
[esw cache]: https://app.cachix.org/organization/ctu-esw/cache/esw#pull
