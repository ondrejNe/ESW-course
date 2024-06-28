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
import java.util.concurrent.atomic.AtomicInteger

object Application : ILoggable {
    private val appLogger = MarkedLogger.markedLogger(javaClass)

    /**
     * Main entry point for the application
     * - Create a thead pool for concurrent processing
     * - Create a server socket
     * - Accept incoming connections
     * - Handle incoming messages
     * - Close the connection
     */
    @JvmStatic
    fun main(args: Array<String>) = runBlocking {
        // Concurrent processing pool
        val threadCount = Runtime.getRuntime().availableProcessors()
        val executor = Executors.newFixedThreadPool(threadCount)
        val dispatcher = executor.asCoroutineDispatcher()

        // Server setup
        val serverSocket = ServerSocket(4321)
        val clientIdCounter = AtomicInteger(1)
        appLogger.info("Dispatcher created with $threadCount threads")
        appLogger.info("Server is running on port ${serverSocket.localPort}")

        // Server start
        val job = launch(dispatcher) {
            while (isActive) {
                // Connection handling
                val clientSocket = serverSocket.accept()
                val clientId = clientIdCounter.getAndIncrement()
                appLogger.info("Client connected: $clientId")
                launch(dispatcher) {
                    handleClient(clientSocket, clientId)
                }
            }
        }

        // Cleanup
        job.join()
        serverSocket.close()
        executor.shutdown()
    }

    private fun handleClient(clientSocket: Socket, id: Int) {
        // IO
        val input = DataInputStream(clientSocket.getInputStream())
        val output = DataOutputStream(clientSocket.getOutputStream())

        try {
            while (true) {
                // Incoming message size
                val size = input.readInt()
                if (size <= 0) {
                    appLogger.info("[$id] Invalid data size received: $size")
                    break
                }

                // Read the complete message
                val dataBuffer = ByteArrayOutputStream(size)
                var totalRead = 0

                while (totalRead < size) {
                    val buffer = ByteArray(size - totalRead)
                    val bytesRead = input.read(buffer)
                    if (bytesRead == -1) { // End of stream reached unexpectedly
                        throw RuntimeException("[$id] Stream ended before expected")
                    }
                    dataBuffer.write(buffer, 0, bytesRead)
                    totalRead += bytesRead
                }

                // Decode & Process
                val request = Request.parseFrom(dataBuffer.toByteArray())
                val response = processRequest(request, id)
                val responseBytes = response.toByteArray()

                // Send response
                output.writeInt(responseBytes.size)
                output.write(responseBytes)
                output.flush()

                // Close connection if OneToAll
                if (request.hasOneToAll()) {
                    appLogger.info("[$id] Closing connection after OneToAll")
                    break
                }
            }
        } catch (e: Exception) {
            appLogger.info("[$id] Exception handling client: ${e.message}")
        } finally {
            appLogger.info("[$id] Closing client socket")
            clientSocket.close()
        }
    }

    private fun processRequest(request: Request, id: Int): Response = when {
        request.hasWalk() -> {
            request.walk.process()
            Response.newBuilder().build()
        }
        request.hasOneToOne() -> {
            appLogger.info("[$id] OneToOne received")
            val result = request.oneToOne.process()
            appLogger.info("[$id] Shortest path length: $result")
            Response.newBuilder().setShortestPathLength(result).build()
        }
        request.hasOneToAll() -> {
            appLogger.info("[$id] OneToAll received")
            val result = request.oneToAll.process()
            appLogger.info("[$id] Total length: $result")
            Response.newBuilder().setTotalLength(result).build()
        }
        request.hasReset() -> {
            appLogger.info("[$id] Reset received")
            request.reset.process()
            Response.newBuilder().build()
        }
        else -> {
            appLogger.info("[$id] Unknown message")
            Response.newBuilder().build()
        }
    }
}
