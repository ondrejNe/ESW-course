## Protobuf

protoc --cpp_out=../src/ scheme.proto

## C++ Server Component

To build the C++ part of the project, install dependencies or use nix shell:

	nix-shell

Then build the project with meson:

	meson setup builddir
	meson compile -C builddir

And finally run the project:

	./builddir/src/server <port>

for example:

    ./builddir/src/server 62420
    