package necasond

import esw.Scheme.OneToAll
import esw.Scheme.OneToOne
import esw.Scheme.Reset
import esw.Scheme.Walk
import java.util.PriorityQueue
import java.util.concurrent.ConcurrentHashMap
import kotlin.math.abs

data class Point(
    val x: Long,
    val y: Long
)

data class Edge(
    val length: Long,
    val samples: Long
)

data class Cell(
    val id: Long,
    val coordX: Long,
    val coordY: Long,
    val pointX: Long,
    val pointY: Long,
    val edges: ConcurrentHashMap<Long, Edge> = ConcurrentHashMap()
)

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

    private val cells: ConcurrentHashMap<Long, Cell> = ConcurrentHashMap()

    fun getPointCellId(point: Point): Long {
        val probableCoordX = point.x / 500
        val probableCoordY = point.y / 500

        // Search whether there isn't a better match
        for (comb in precomputedNeighbourPairs) {
            val neighborCellId = ((probableCoordX + comb.first) shl 32) or (probableCoordY + comb.second)
            val cell = cells[neighborCellId]

            cell?.let {
                val dx = abs(point.x - it.pointX)
                val dy = abs(point.y - it.pointY)
                if ((dx * dx + dy * dy) <= 250000) {
                    return neighborCellId
                }
            }
        }

        return ((probableCoordX shl 32) or probableCoordY)
    }

    fun addEdge(originCellId: Long, destinationCellId: Long, length: Long) {
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

    fun addPoint(point: Point, cellId: Long) {
        cells.getOrPut(cellId) {
            Cell(
                id = cellId,
                pointX = point.x,
                pointY = point.y,
                coordX = point.x / 500,
                coordY = point.y / 500,
                edges = ConcurrentHashMap()
            )
        }
    }

    fun resetGrid() {
        cells.clear()
    }

    private fun djikstra(originCellId: Long, destinationCellId: Long, oneToAll: Boolean = false): Long {
        val visited = HashSet<Long>()
        var sum = 0L
        val pq = PriorityQueue<Pair<Long, Long>> { a, b -> a.first.compareTo(b.first) }

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

                pq.add((originCurrent + edge.length / edge.samples) to neighborCellId)
            }

        }

        return sum
    }

    fun Reset.process() {
        resetGrid()
    }

    fun Walk.process() {
        (1..lengthsCount).forEach { index ->
            val origin = getLocations(index-1).let { Point(it.x.toLong(), it.y.toLong()) }
            val originCellId = getPointCellId(origin)
            addPoint(origin, originCellId)

            val destination = getLocations(index).let { Point(it.x.toLong(), it.y.toLong()) }
            val destinationCellId = getPointCellId(destination)
            addPoint(destination, destinationCellId)

            val length = getLengths(index - 1).toLong()
            addEdge(originCellId, destinationCellId, length)
        }
    }

    fun OneToOne.process(): Long {
        val originCellId = getPointCellId(Point(origin.x.toLong(), origin.y.toLong()))
        val destinationCellId = getPointCellId(Point(destination.x.toLong(), destination.y.toLong()))

        val shortestPath = djikstra(originCellId, destinationCellId)

        return shortestPath
    }

    fun OneToAll.process(): Long {
        val originCellId = getPointCellId(Point(origin.x.toLong(), origin.y.toLong()))

        val shortestPath = djikstra(originCellId, originCellId, true)

        return shortestPath
    }
}
