PARALLEL := 1

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

build-clean: clean build

run-server:
	./build/src/efficient_server

## Build the docker image -----------------------------------------------------
docker-build:
	docker build -t server/esw .

## Mount host file system and run the docker image
docker-run:
	docker run --rm -it -v "$(PWD):/home/dev" --name server-esw server/esw:latest
