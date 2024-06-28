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

    @JvmStatic
    fun main(args: Array<String>) = runBlocking {
        val threadCount = Runtime.getRuntime().availableProcessors()
        val executor = Executors.newFixedThreadPool(threadCount)
        val dispatcher = executor.asCoroutineDispatcher()

        val serverSocket = ServerSocket(4321)
        val clientIdCounter = AtomicInteger(1)
        appLogger.info("Dispatcher created with $threadCount threads")
        appLogger.info("Server is running on port ${serverSocket.localPort}")
        
        val job = launch(dispatcher) {
            while (isActive) {
                val clientSocket = serverSocket.accept()
                val clientId = clientIdCounter.getAndIncrement()
                appLogger.info("Client connected: $clientId")
                launch(dispatcher) {
                    handleClient(clientSocket, clientId)
                }
            }
        }

        job.join()
        serverSocket.close()
        executor.shutdown()
    }

    private fun handleClient(clientSocket: Socket, id: Int) {
        val input = DataInputStream(clientSocket.getInputStream())
        val output = DataOutputStream(clientSocket.getOutputStream())

        try {
            while (true) {
                // Read the size of the upcoming message
                val size = input.readInt()
                if (size <= 0) {
                    appLogger.debug("[$id] Invalid data size received: $size")
                    break
                }

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

                // Decode the complete Protobuf message
                val data = Request.parseFrom(dataBuffer.toByteArray())
                val response = processRequest(data, id)
                val responseBytes = response.toByteArray()
                output.writeInt(responseBytes.size)
                output.write(responseBytes)
                output.flush()

                if (data.hasOneToAll()) {
                    appLogger.debug("[$id] Closing connection after OneToAll")
                    break
                }
            }
        } catch (e: Exception) {
            appLogger.debug("[$id] Exception handling client: ${e.message}")
        } finally {
            appLogger.debug("[$id] Closing client socket")
            clientSocket.close()
        }
    }

    private fun processRequest(data: Request, id: Int): Response = when {
        data.hasWalk() -> {
            data.walk.process()
            Response.newBuilder().build()
        }
        data.hasOneToOne() -> {
            appLogger.debug("[$id] OneToOne received")
            val result = data.oneToOne.process()
            appLogger.debug("[$id] Shortest path length: $result")
            Response.newBuilder().setShortestPathLength(result).build()
        }
        data.hasOneToAll() -> {
            appLogger.debug("[$id] OneToAll received")
            val result = data.oneToAll.process()
            appLogger.debug("[$id] Total length: $result")
            Response.newBuilder().setTotalLength(result).build()
        }
        data.hasReset() -> {
            appLogger.debug("[$id] Reset received")
            data.reset.process()
            Response.newBuilder().build()
        }
        else -> {
            appLogger.debug("[$id] Unknown message")
            Response.newBuilder().build()
        }
    }
}
