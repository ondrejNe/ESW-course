# Proto class generation

```sh
cd ./src/main/resources && mkdir -p ./java && protoc -I=./ --java_out=./java ./scheme.proto
```

# How to run
    
```sh
./gradlew run
```
