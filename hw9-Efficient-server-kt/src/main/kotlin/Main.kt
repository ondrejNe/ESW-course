package necasond

import esw.Scheme.Request
import esw.Scheme.Response
import kotlinx.coroutines.asCoroutineDispatcher
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import necasond.GridData.process
import java.io.ByteArrayOutputStream
import java.io.DataInputStream
import java.io.DataOutputStream
import java.net.ServerSocket
import java.net.Socket
import java.util.concurrent.Executors

fun main() = runBlocking {
    val threadCount = 4
    val executor = Executors.newFixedThreadPool(threadCount)
    val dispatcher = executor.asCoroutineDispatcher()

    val serverSocket = ServerSocket(4321)
    println("Server is running on port 4321")

    val job = launch(dispatcher) {
        while (isActive) {
            val clientSocket = serverSocket.accept()
            launch(dispatcher) {
                handleClient(clientSocket)
            }
        }
    }

    job.join()
    serverSocket.close()
    executor.shutdown()
}

fun handleClient(clientSocket: Socket) {
    val input = DataInputStream(clientSocket.getInputStream())
    val output = DataOutputStream(clientSocket.getOutputStream())

    try {
        while (true) {
            // Read the size of the upcoming message
            val size = input.readInt()
            if (size <= 0) {
                println("Invalid data size received: $size")
                break
            }

            val dataBuffer = ByteArrayOutputStream(size)
            var totalRead = 0

            while (totalRead < size) {
                val buffer = ByteArray(size - totalRead)
                val bytesRead = input.read(buffer)
                if (bytesRead == -1) { // End of stream reached unexpectedly
                    throw RuntimeException("Stream ended before expected")
                }
                dataBuffer.write(buffer, 0, bytesRead)
                totalRead += bytesRead
            }

            // Decode the complete Protobuf message
            val data = Request.parseFrom(dataBuffer.toByteArray())
            val response = processRequest(data)
            val responseBytes = response.toByteArray()
            output.writeInt(responseBytes.size)
            output.write(responseBytes)
            output.flush()

            if (data.hasOneToAll()) {
                println("Closing connection after OneToAll")
                break
            }
        }
    } catch (e: Exception) {
        println("Exception handling client: ${e.message}")
    } finally {
        println("Closing client socket")
        clientSocket.close()
    }
}

fun processRequest(data: Request): Response = when {
    data.hasWalk() -> {
        data.walk.process()
        Response.newBuilder().build()
    }
    data.hasOneToOne() -> {
        println("OneToOne received")
        val result = data.oneToOne.process()
        println("Shortest path length: $result")
        Response.newBuilder().setShortestPathLength(result).build()
    }
    data.hasOneToAll() -> {
        println("OneToAll received")
        val result = data.oneToAll.process()
        println("Total length: $result")
        Response.newBuilder().setTotalLength(result).build()
    }
    data.hasReset() -> {
        println("Reset received")
        data.reset.process()
        Response.newBuilder().build()
    }
    else -> {
        println("Unknown message")
        Response.newBuilder().build()
    }
}

