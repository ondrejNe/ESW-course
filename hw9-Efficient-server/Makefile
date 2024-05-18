PARALLEL := 8

## Make driven command interface to simplify RPI Pico development in Docker
.PHONY: docker-build docker-run

## Build the project ----------------------------------------------------------
all:
	mkdir -p build
	cmake -S ./ -B ./build/

build: all
	cmake --build ./build --parallel $(PARALLEL)

clean:
	rm -rf build

clean-build: clean build

## Run the project ------------------------------------------------------------
run-server:
	./build/server-src/efficient_server 1234

run-test-one:
	nc localhost 1234 < test/walk1nodes3-2.pbf

run-test-two:
	nc localhost 1234 < test/walk1nodes100-2.pbf

run-test-three:
	nc localhost 1234 < test/walk2nodes6.pbf

run-test-four:
	nc localhost 1234 < test/walk3000.pbf

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
