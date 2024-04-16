### Avro

    avrogencpp -i measurements.avsc -o ../cpp/measurements.av.h -n c

    export PATH=$PATH:/nix/store/0zihsc1dzpbdzidm052zl765hg3rc3hz-avro-c++-1.11.0/bin
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nix/store/0zihsc1dzpbdzidm052zl765hg3rc3hz-avro-c++-1.11.0/lib

### Protobuf

    protoc --cpp_out=../cpp ./measurements.proto
    protoc --java_out=./java ./measurements.proto

### C++ Server Component

To build the C++ part of the project, install dependencies or use nix shell:

	nix-shell

Then build the project with meson:

	meson --buildtype=plain builddir

    cd builddir
    ninja

    meson compile -C builddir


And finally run the project:

	./builddir/src/main/cpp/server <port> <format>

for example:

    ./builddir/src/main/cpp/server 12345 json

### Java Client Component

To build the Java part of the project, install JDK 17 or higher or use nix shell:

    nix-shell

Then build the project using maven:

    mvn compile

And run it:

    mvn exec:java -Dexec.mainClass="cz.esw.serialization.App" -Dexec.args="<host> <port> <format>"

for example:

    mvn exec:java -Dexec.mainClass="cz.esw.serialization.App" -Dexec.args="localhost 12345 json"

Or use any IDE and import it as a maven project and to see how it works for JSON run `AppTest.java`.
