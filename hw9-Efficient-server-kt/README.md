# Application purpose
The application is designed to efficiently calculate the shortest drivable distances between any two locations in a city using data received from vehicles. This data, which includes distances traveled and specific locations, is collected and processed by the server to construct a directed graph. Each node in the graph represents a physical location, and the edges represent the traveled paths between these locations.

The server leverages a TCP and Protobuf-based communication system to handle a high volume of concurrent client requests and data transmissions. It processes incoming data to update the graph dynamically.

## Description of algorithms and techniques
Connections are managed by the java.net Sockets and handled by the coroutine ThreadPool. Data storage is secured by the 
ConcurrentHashMap on which concurrent reads and writes are performed. The graph is represented by the adjacency list. 
The Dijkstra algorithm is used to find the shortest path between two nodes. Whole project is written in Kotlin and
managed by Gradle.

## Compile & run instructions
```shell
java --version
```

To run the server on port 4321
```shell
./gradlew run
```

## Used libraries
See `libs.versions.toml`

## Protobuf resource
```shell
cd ./src/main/resources && mkdir -p ./java && protoc -I=./ --java_out=./java ./scheme.proto
```
