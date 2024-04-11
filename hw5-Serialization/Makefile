
.PHONY: docker-build docker-run

## Build the docker image
docker-build:
	docker build -t esw/serialization .

## Mount host file system and run the docker image
docker-run:
	docker run --rm -it -v "$(PWD):/home/dev" --name esw-serialization esw/serialization:latest

#json-server:
#	nix-shell --run 'meson setup builddir'
#    nix-shell --run 'meson compile -C builddir'
#	nix-shell --run ./builddir/src/main/cpp/server 12345 json
#
#json-client:
#	mvn exec:java -Dexec.mainClass="cz.esw.serialization.App" -Dexec.args="localhost 12345 json"
#
#proto-server:
#	./builddir/src/main/cpp/server 12345 proto
#
#proto-client:
#	mvn exec:java -Dexec.mainClass="cz.esw.serialization.App" -Dexec.args="localhost 12345 proto"
#
#avro-server:
#	./builddir/src/main/cpp/server 12345 avro
#
#avro-client:
#	mvn exec:java -Dexec.mainClass="cz.esw.serialization.App" -Dexec.args="localhost 12345 avro"