package necasond

import net.logstash.logback.marker.LogstashMarker
import net.logstash.logback.marker.Markers
import net.logstash.logback.marker.Markers.append
import org.slf4j.Logger
import org.slf4j.LoggerFactory
import org.slf4j.Marker
import java.time.Instant

interface ILoggable

fun ILoggable.logger(): Logger = logger(this.javaClass)

private fun <T : ILoggable> logger(forClass: Class<T>): Logger =
    loggerForName(forClass.name.removeSuffix("\$Companion"))

private fun loggerForName(forName: String) = LoggerFactory.getLogger(forName)

/**
 * Top level method for instantiating logger in a top level function that does not belong to any class
 */
fun logger(name: String): Logger = loggerForName(name)

inline fun Logger.debug(lazyMessage: () -> String) { if (isDebugEnabled) debug(lazyMessage()) }
inline fun Logger.trace(lazyMessage: () -> String) { if (isTraceEnabled) trace(lazyMessage()) }
inline fun Logger.info(lazyMessage: () -> String) { if (isInfoEnabled) info(lazyMessage()) }
inline fun Logger.warn(lazyMessage: () -> String) { if (isWarnEnabled) warn(lazyMessage()) }
inline fun Logger.error(lazyMessage: () -> String) { if (isErrorEnabled) error(lazyMessage()) }

inline fun Logger.debug(t: Throwable, lazyMessage: () -> String) { if (isDebugEnabled) debug(lazyMessage(), t) }
inline fun Logger.trace(t: Throwable, lazyMessage: () -> String) { if (isTraceEnabled) trace(lazyMessage(), t) }
inline fun Logger.info(t: Throwable, lazyMessage: () -> String) { if (isInfoEnabled) info(lazyMessage(), t) }
inline fun Logger.warn(t: Throwable, lazyMessage: () -> String) { if (isWarnEnabled) warn(lazyMessage(), t) }
inline fun Logger.error(t: Throwable, lazyMessage: () -> String) { if (isErrorEnabled) error(lazyMessage(), t) }

class MarkedLogger(private val logger: Logger, private val marker: Marker = Markers.empty()) {
    fun info(message: String, lazyMarkers: () -> Marker = { Markers.empty() }) {
        if (logger.isInfoEnabled) logger.info(Markers.aggregate(marker, lazyMarkers()), message)
    }

    fun debug(message: String, lazyMarkers: () -> Marker = { Markers.empty() }) {
        if (logger.isDebugEnabled) logger.debug(Markers.aggregate(marker, lazyMarkers()), message)
    }

    fun debug(message: String, ex: Throwable, lazyMarkers: () -> Marker = { Markers.empty() }) {
        if (logger.isDebugEnabled) logger.debug(Markers.aggregate(marker, lazyMarkers()), message, ex)
    }

    fun warn(message: String, lazyMarkers: () -> Marker = { Markers.empty() }) {
        if (logger.isWarnEnabled) logger.warn(Markers.aggregate(marker, lazyMarkers()), message)
    }

    fun warn(message: String, ex: Throwable, lazyMarkers: () -> Marker = { Markers.empty() }) {
        if (logger.isWarnEnabled) logger.warn(Markers.aggregate(marker, lazyMarkers()), message, ex)
    }

    fun error(message: String, lazyMarkers: () -> Marker = { Markers.empty() }) {
        if (logger.isErrorEnabled) logger.error(Markers.aggregate(marker, lazyMarkers()), message)
    }

    fun error(message: String, ex: Throwable, lazyMarkers: () -> Marker = { Markers.empty() }) {
        if (logger.isErrorEnabled) logger.error(Markers.aggregate(marker, lazyMarkers()), message, ex)
    }

    companion object {
        fun markedLogger(clazz: Class<Any>, defaultMarker: Marker = Markers.empty()): MarkedLogger =
            MarkedLogger(LoggerFactory.getLogger(clazz), defaultMarker)
    }
}

fun markByCurrentTime(current: Instant): LogstashMarker = append("current_time", current)

fun markBy(fieldName: String, anyObject: Any?): LogstashMarker = append(fieldName, anyObject)

fun LogstashMarker.andBy(fieldName: String, anyObject: Any?): LogstashMarker = and(append(fieldName, anyObject))
