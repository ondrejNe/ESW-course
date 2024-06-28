package necasond

import esw.Scheme.Request
import esw.Scheme.Response
import kotlinx.coroutines.asCoroutineDispatcher
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import necasond.GridData.process
import java.io.DataInputStream
import java.io.DataOutputStream
import java.net.ServerSocket
import java.net.Socket
import java.util.concurrent.Executors

fun main() = runBlocking {
    val threadCount = 30
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

    while (true) {
        val size = input.readInt()
        val byteArray = ByteArray(size)
        input.readFully(byteArray)  // Read the Protobuf message

        // Decode the Protobuf message
        val data = Request.parseFrom(byteArray)

        val response = when {
            data.hasWalk() -> {
                data.walk.process()
                Response.newBuilder().build()
            }
            data.hasOneToOne() -> {
                println("OneToOne received")
                val result = data.oneToOne.process()
                Response.newBuilder().setShortestPathLength(result).build()
            }
            data.hasOneToAll() -> {
                println("OneToAll received")
                val result = data.oneToAll.process()
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

        val responseBytes = response.toByteArray()
        output.writeInt(responseBytes.size)
        output.write(responseBytes)
        output.flush()

        if (data.hasOneToAll()) {
            clientSocket.close()
            break
        }
    }
}
