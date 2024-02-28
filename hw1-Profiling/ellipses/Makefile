

.PHONY: docker-build docker-run

## Build the docker image
docker-build:
	docker build -t esw/ellipses .

## Mount host file system and run the docker image
docker-run:
	docker run --rm -it -v "$(PWD):/home/dev" --name eswDir esw/ellipses:latest
