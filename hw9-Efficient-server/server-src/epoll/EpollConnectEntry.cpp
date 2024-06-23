
#include "EpollConnectEntry.hh"

// Global variables -------------------------------------------------------------------------------
//#define CONNECT_LOGGER
//#define PROCESS_LOGGER
PrefixedLogger connectLogger = PrefixedLogger("[CONNECTION]", true);

// Class definition -------------------------------------------------------------------------------
bool EpollConnectEntry::handleEvent(uint32_t events) {
    if (!this->is_fd_valid()) {
#ifdef CONNECT_LOGGER
        connectLogger.error("Invalid file descriptor [FD%d]", this->get_fd());
#endif
        return false;
    } else if (events & EPOLLERR) {
        // Retrieve the specific error code
        int err = 0;
        socklen_t len = sizeof(err);
        if (getsockopt(this->get_fd(), SOL_SOCKET, SO_ERROR, &err, &len) == 0) {
#ifdef CONNECT_LOGGER
            connectLogger.error("EPOLLERR received on connection [FD%d]: %s", this->get_fd(), strerror(err));
#endif
        } else {
#ifdef CONNECT_LOGGER
            connectLogger.error("EPOLLERR received on connection [FD%d]: Unable to retrieve error code",
                                this->get_fd());
#endif
        }
        return false;
    } else if (events & EPOLLHUP) {
#ifdef CONNECT_LOGGER
        connectLogger.warn("EPOLLHUP received on connection [FD%d]", this->get_fd());
#endif
        return false;
    } else if (events & EPOLLRDHUP) {
#ifdef CONNECT_LOGGER
        connectLogger.error("EPOLLRDHUP received on connection [FD%d]", this->get_fd());
#endif
        return false;
    } else if (events & EPOLLIN) {
        try {
            readEvent();
        }
        catch (exception &e) {
#ifdef CONNECT_LOGGER
            connectLogger.error("readEvent(): %s on connection [FD%d]", e.what(), this->get_fd());
#endif
            return false;
        }
    }

    return true; // Keep the connection going
}

void EpollConnectEntry::readEvent() {
    if (processingInProgress) {
        return;
    }

    // New message
    if (!messageInProgress) {
        inProgressMessageSize = readMessageSize();
        if (inProgressMessageSize < 0) {
#ifdef CONNECT_LOGGER
            connectLogger.warn("Failed to read message size on connection [FD%d]", this->get_fd());
#endif
            shutdown(this->get_fd(), SHUT_RDWR);
            return;
        }
        messageInProgress = true;
        inProgressMessageOffset = 0;
    }

    // Read the message
    int received = recv(this->get_fd(), messageBuffer + inProgressMessageOffset,
                        inProgressMessageSize - inProgressMessageOffset, MSG_WAITALL);
#ifdef CONNECT_LOGGER
    connectLogger.debug("Received: %d on connection [FD%d]", received, this->get_fd());
#endif
    if (received == 0) {
#ifdef CONNECT_LOGGER
        connectLogger.debug("Connection closed by client on [FD%d]", this->get_fd());
#endif
        shutdown(this->get_fd(), SHUT_RDWR);
        return;
    }

    if (received < 0) {
#ifdef CONNECT_LOGGER
        connectLogger.error("Failed to read message: %s on [FD%d]", string(strerror(errno)), this->get_fd());
#endif
        shutdown(this->get_fd(), SHUT_RDWR);
        return;
    }
    inProgressMessageOffset += received;

    if (inProgressMessageOffset != inProgressMessageSize) {
#ifdef CONNECT_LOGGER
        connectLogger.debug("Received %d of %d on connection [FD%d]", received, inProgressMessageSize, this->get_fd());
#endif
        return;
    }

    // All the message was read
    esw::Request request;
    esw::Response response;
    response.set_status(esw::Response_Status_OK);
    request.ParseFromArray(messageBuffer, inProgressMessageSize);

#ifdef CONNECT_LOGGER
    connectLogger.debug("Message handed to processing on connection [FD%d]", this->get_fd());
#endif
    processingInProgress = true;
    int fd = this->get_fd();

    if (request.has_walk() || request.has_reset()) {
        processMessage(request, response, gridData, gridStats, fd);
        this->processingInProgress = false;
//        resourcePool.run([this, request, response, &gridData, &gridStats, fd] {
//            processMessage(request, response, gridData, gridStats, fd);
//            this->processingInProgress = false;
//        }, fd);
    } else {
//        processMessage(request, response, gridData, gridStats, fd);
//        this->processingInProgress = false;
        resourcePool1.run([this, request, response, &gridData, &gridStats, fd] {
            processMessage(request, response, gridData, gridStats, fd);
            this->processingInProgress = false;
        }, fd);
    }

    messageInProgress = false;
}

int EpollConnectEntry::readMessageSize() {
    int msgSize;
    char sizeBytes[4];
    int received = recv(this->get_fd(), sizeBytes, sizeof(sizeBytes), MSG_WAITALL);

    if (received == 0) {
#ifdef CONNECT_LOGGER
        connectLogger.debug("Connection closed by client on [FD%d]", this->get_fd());
#endif
        return -1; // signifying that the connection should be closed
    }

    if (received != 4) {
#ifdef CONNECT_LOGGER
        connectLogger.error("On size read expected 4 got: %d on connection [FD%d]", received, this->get_fd());
#endif
        esw::Response response;
        response.set_status(esw::Response_Status_OK);
        writeResponse(response, this->get_fd());
        return -1; // signifying that the connection should be closed
    }

    memcpy(&msgSize, sizeBytes, sizeof(int));
    // Converts u_long from TCP/IP network order to host byte order
    msgSize = ntohl(msgSize);
#ifdef CONNECT_LOGGER
    connectLogger.debug("Expecting: %d on connection [FD%d]", msgSize, this->get_fd());
#endif
    return msgSize;
}

void EpollConnectEntry::processMessage(esw::Request request, esw::Response response, GridData &gridData, GridStats &gridStats, int fd) {
    if (request.has_walk()) {
#ifdef PROCESS_LOGGER
        connectLogger.warn("Walk message received on connection [FD%d]", fd);
#endif
        const esw::Walk &walk = request.walk();
        processWalk(gridData, gridStats, walk);

    } else if (request.has_onetoone()) {
#ifdef PROCESS_LOGGER
        connectLogger.warn("OneToOne message received on connection [FD%d]", fd);
#endif
        const esw::OneToOne &oneToOne = request.onetoone();
        uint64_t val = processOneToOne(gridData, gridStats, oneToOne);
#ifdef PROCESS_LOGGER
        connectLogger.info("OneToOne response %llu on connection [FD%d]", val, fd);
#endif
        response.set_shortest_path_length(val);

    } else if (request.has_onetoall()) {
#ifdef PROCESS_LOGGER
        connectLogger.warn("OneToAll message received on connection [FD%d]", fd);
#endif
        const esw::OneToAll &oneToAll = request.onetoall();
        uint64_t val = processOneToAll(gridData, gridStats, oneToAll);
#ifdef PROCESS_LOGGER
        connectLogger.info("OneToAll response %llu on connection [FD%d]", val, fd);
#endif
        response.set_total_length(val);

    } else if (request.has_reset()) {
#ifdef PROCESS_LOGGER
        connectLogger.warn("Reset message received on connection [FD%d]", fd);
#endif
        processReset(gridData, gridStats);

    } else {
#ifdef PROCESS_LOGGER
        connectLogger.error("No valid message type detected on connection [FD%d]", fd);
#endif
        response.set_status(esw::Response_Status_ERROR);
    }

    // Send the response
    writeResponse(response, fd);

    // Final request should close the connection
    if (request.has_onetoall()) {
        shutdown(fd, SHUT_RDWR);
#ifdef PROCESS_LOGGER
        connectLogger.info("Closing connection after OneToAll request on connection [FD%d]", fd);
#endif
    }
}

void EpollConnectEntry::writeResponse(esw::Response &response, int fd) {
    // Get the size of the serialized response
    size_t size = response.ByteSizeLong();
#ifdef PROCESS_LOGGER
    connectLogger.debug("Response size: %d status: %d on connection [FD%d]", size, response.status(), fd);
#endif
    // Convert the response size to network byte order
    int networkByteOrderSize = htonl(size);

    int bytesSent = send(fd, (const char *) (&networkByteOrderSize), 4, 0);
    if (bytesSent < 0) {
#ifdef PROCESS_LOGGER
        connectLogger.error("Failed to send response size on connection [FD%d]: %s", fd, string(strerror(errno)));
#endif
    } else {
        response.SerializeToFileDescriptor(fd);
        fsync(fd);
    }
}
