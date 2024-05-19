PARALLEL := 8

## Make driven command interface to simplify RPI Pico development in Docker
.PHONY: docker-build docker-run

## Build the project ----------------------------------------------------------
all:
	mkdir -p build

build: all
	cmake -S ./ -B ./build/
	cmake --build ./build --parallel $(PARALLEL)

debug: all
	cmake -DCMAKE_BUILD_TYPE=Debug -S ./ -B ./build/
	cmake --build ./build --parallel $(PARALLEL)

clean:
	rm -rf build

clean-build: clean build

clean-debug: clean debug

## Run the project ------------------------------------------------------------
run-server:
	./build/server-src/efficient_server 4321

gdb-server:
	gdb ./build/server-src/efficient_server

perf-server:
	perf record ./build/server-src/efficient_server

run-test-one:
	nc localhost 4321 < test/walk1nodes3-2.pbf

run-test-two:
	nc localhost 4321 < test/walk1nodes100-2.pbf

run-test-three:
	nc localhost 4321 < test/walk2nodes6.pbf

run-test-four:
	nc localhost 4321 < test/walk3000.pbf

## Project environment --------------------------------------------------------
## Build the docker image
docker-build:
	docker build -t server/esw .

## Mount host file system and run the docker image
docker-run:
	docker run --rm -it -v "$(PWD):/home/dev" --name server-esw server/esw:latest

## Connect to the running container in a separate bash session
docker-connect:
	docker exec -it server-esw /bin/bash
