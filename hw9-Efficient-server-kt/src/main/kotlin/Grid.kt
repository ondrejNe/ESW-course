package necasond

import esw.Scheme.OneToAll
import esw.Scheme.OneToOne
import esw.Scheme.Reset
import esw.Scheme.Walk
import java.util.PriorityQueue
import java.util.concurrent.ConcurrentHashMap
import kotlin.math.abs

/**
 * @param length Cumulative length of the path from samples
 * @param samples Number of samples
 */
data class Edge(
    val length: Long,
    val samples: Long,
)

/**
 * @param id Unique identifier for the cell
 * @param coordX
 * @param coordY
 * @param pointX Root point
 * @param pointY Root point
 * @param edges Neighbouring cells
 */
data class Cell(
    val id: Long,
    val coordX: Long,
    val coordY: Long,
    val pointX: Long,
    val pointY: Long,
    val edges: ConcurrentHashMap<Long, Edge> = ConcurrentHashMap(5)
)

/**
 * Grid data structure for storing the grid cells
 *
 * @property cells Main data structure for storing the grid cells
 */
@Suppress("unused")
object GridData {
    private val precomputedNeighbourPairs = listOf(
        Pair(-1L, -1L),
        Pair(-1L, 0L),
        Pair(-1L, 1L),
        Pair(0L, -1L),
        Pair(0L, 1L),
        Pair(1L, -1L),
        Pair(1L, 0L),
        Pair(1L, 1L)
    )

    // Most Valuable Player
    private val cells: ConcurrentHashMap<Long, Cell> = ConcurrentHashMap(115000)

    // Get the cell id for a given point
    private fun getPointCellId(pointX: Long, pointY: Long): Long {
        val probableCoordX = pointX / 500
        val probableCoordY = pointY / 500

        // Search whether there isn't a better match
        for (comb in precomputedNeighbourPairs) {
            val neighborCellId = ((probableCoordX + comb.first) shl 32) or (probableCoordY + comb.second)
            val cell = cells[neighborCellId]

            cell?.let {
                val dx = abs(pointX - it.pointX)
                val dy = abs(pointY - it.pointY)
                if ((dx * dx + dy * dy) <= 250000) {
                    return neighborCellId
                }
            }
        }

        return ((probableCoordX shl 32) or probableCoordY)
    }

    // Add an edge between two cells
    private fun addEdge(originCellId: Long, destinationCellId: Long, length: Long) {
        cells.getValue(originCellId).edges.compute(destinationCellId) { _, oldValue ->
            when {
                oldValue == null -> Edge(length, 1L)
                else -> oldValue.copy(
                    length = oldValue.length + length,
                    samples = oldValue.samples + 1L,
                )
            }
        }
    }

    // Add a point to the grid
    private fun addPoint(pointX: Long, pointY: Long, cellId: Long) {
        cells.getOrPut(cellId) {
            Cell(
                id = cellId,
                pointX = pointX,
                pointY = pointY,
                coordX = pointX / 500,
                coordY = pointY / 500,
                edges = ConcurrentHashMap()
            )
        }
    }

    // Reset the grid
    private fun resetGrid() {
        cells.clear()
    }

    // Djikstra algorithm for the shortest paths
    private fun djikstra(originCellId: Long, destinationCellId: Long, oneToAll: Boolean = false): Long {
        val visited = HashSet<Long>(115000)
        var sum = 0L
        val pq = PriorityQueue<Pair<Long, Long>>(300) { a, b -> a.first.compareTo(b.first) }

        pq.add(0L to originCellId)

        while (pq.isNotEmpty()) {
            val (originCurrent, currentCellId) = pq.peek()
            pq.remove()

            if (visited.contains(currentCellId)) continue
            visited.add(currentCellId)

            if (currentCellId == destinationCellId && !oneToAll) {
                sum = originCurrent
                break
            } else {
                sum += originCurrent
            }

            cells.getValue(currentCellId).edges.forEach { (neighborCellId, edge) ->
                if (visited.contains(neighborCellId)) return@forEach
                val neighborDistance = (originCurrent + (edge.length / edge.samples))

                pq.add(neighborDistance to neighborCellId)
            }

        }

        return sum
    }

    /**
     * Shared public API methods for the grid
     */
    fun Reset.process() {
        resetGrid()
    }

    /**
     * Shared public API methods for the grid
     */
    fun Walk.process() {
        (1..lengthsCount).forEach { index ->
            val origin = getLocations(index - 1)
            val originCellId = getPointCellId(origin.x.toLong(), origin.y.toLong())
            addPoint(origin.x.toLong(), origin.y.toLong(), originCellId)

            val destination = getLocations(index)
            val destinationCellId = getPointCellId(destination.x.toLong(), destination.y.toLong())
            addPoint(destination.x.toLong(), destination.y.toLong(), destinationCellId)

            val length = getLengths(index - 1).toLong()
            addEdge(originCellId, destinationCellId, length)
        }
    }

    /**
     * Shared public API methods for the grid
     */
    fun OneToOne.process(): Long {
        val originCellId = getPointCellId(origin.x.toLong(), origin.y.toLong())
        val destinationCellId = getPointCellId(destination.x.toLong(), destination.y.toLong())

        val shortestPath = djikstra(originCellId, destinationCellId)

        return shortestPath
    }

    /**
     * Shared public API methods for the grid
     */
    fun OneToAll.process(): Long {
        val originCellId = getPointCellId(origin.x.toLong(), origin.y.toLong())

        val shortestPath = djikstra(originCellId, originCellId, true)

        return shortestPath
    }
}
