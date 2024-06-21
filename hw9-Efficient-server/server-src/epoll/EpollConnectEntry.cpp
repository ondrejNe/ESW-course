
#include "EpollConnectEntry.hh"

// Global variables -------------------------------------------------------------------------------
PrefixedLogger connectLogger = PrefixedLogger("[CONNECTION]", true);
PrefixedLogger processLogger = PrefixedLogger("[PROCESSING]", true);

// Class definition -------------------------------------------------------------------------------
bool EpollConnectEntry::handleEvent(uint32_t events) {
    if (!this->is_fd_valid()) {
        connectLogger.error("Invalid file descriptor [FD%d]", this->get_fd());
        return false;
    }
    else if (events & EPOLLERR) {
        // Retrieve the specific error code
        int err = 0;
        socklen_t len = sizeof(err);
        if (getsockopt(this->get_fd(), SOL_SOCKET, SO_ERROR, &err, &len) == 0) {
            connectLogger.error("EPOLLERR received on connection [FD%d]: %s", this->get_fd(), strerror(err));
        } else {
            connectLogger.error("EPOLLERR received on connection [FD%d]: Unable to retrieve error code", this->get_fd());
        }
        return false;
    }
    else if (events & EPOLLHUP) {
        connectLogger.warn("EPOLLHUP received on connection [FD%d]", this->get_fd());
        return false;
    }
    else if (events & EPOLLRDHUP) {
        connectLogger.error("EPOLLRDHUP received on connection [FD%d]", this->get_fd());
        return false;
    }
    else if (events & EPOLLIN) {
        try {
            readEvent();
        }
        catch (exception &e) {
            connectLogger.error("readEvent(): %s on connection [FD%d]", e.what(), this->get_fd());
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
    inProgressMessageSize = readMessageSize();
    if (inProgressMessageSize < 0) {
        connectLogger.warn("Failed to read message size on connection [FD%d]", this->get_fd());
        shutdown(this->get_fd(), SHUT_RDWR);
        return;
    }

    // Read the message
    int received = recv(this->get_fd(), messageBuffer, inProgressMessageSize, MSG_WAITALL);
    connectLogger.debug("Received: %d on connection [FD%d]", received, this->get_fd());

    if (received == 0) {
        connectLogger.debug("Connection closed by client on [FD%d]", this->get_fd());
        shutdown(this->get_fd(), SHUT_RDWR);
        return;
    }

    if (received < 0) {
        connectLogger.error("Failed to read message: %s on [FD%d]", string(strerror(errno)), this->get_fd());
        shutdown(this->get_fd(), SHUT_RDWR);
        return;
    }

    if (received != inProgressMessageSize) {
        connectLogger.error("Received %d of %d on connection [FD%d]", received, inProgressMessageSize, this->get_fd());
        shutdown(this->get_fd(), SHUT_RDWR);
        return;
    }

    // All the message was read
    esw::Request request;
    esw::Response response;
    response.set_status(esw::Response_Status_OK);
    request.ParseFromArray(messageBuffer, inProgressMessageSize);

    connectLogger.info("Message handed to processing on connection [FD%d]", this->get_fd());
    processingInProgress = true;
    int fd = this->get_fd();
    resourcePool.run([this, request, response, fd] {
        processMessage(request, response, fd);
        this->processingInProgress = false;
    }, fd);
}

int EpollConnectEntry::readMessageSize() {
    int msgSize;
    char sizeBytes[4];
    int received = recv(this->get_fd(), sizeBytes, sizeof(sizeBytes), MSG_WAITALL);

    if (received == 0) {
        connectLogger.debug("Connection closed by client on [FD%d]", this->get_fd());
        return -1; // signifying that the connection should be closed
    }

    if (received != 4) {
        connectLogger.error("On size read expected 4 got: %d on connection [FD%d]", received, this->get_fd());
        esw::Response response;
        response.set_status(esw::Response_Status_OK);
        writeResponse(response, this->get_fd());
        return -1; // signifying that the connection should be closed
    }

    memcpy(&msgSize, sizeBytes, sizeof(int));
    // Converts u_long from TCP/IP network order to host byte order
    msgSize = ntohl(msgSize);
    connectLogger.debug("Expecting: %d on connection [FD%d]", msgSize, this->get_fd());
    return msgSize;
}

void EpollConnectEntry::processMessage(esw::Request request, esw::Response response, int fd) {
    if (request.has_walk()) {
        processLogger.warn("Walk message received on connection [FD%d]", fd);
        const esw::Walk &walk = request.walk();
        grid.processWalk(walk);

    } else if (request.has_onetoone()) {
        processLogger.warn("OneToOne message received on connection [FD%d]", fd);
        const esw::OneToOne &oneToOne = request.onetoone();
        uint64_t val = grid.processOneToOne(oneToOne);

        processLogger.info("OneToOne response %llu on connection [FD%d]", val, fd);
        response.set_shortest_path_length(val);

    } else if (request.has_onetoall()) {
        processLogger.warn("OneToAll message received on connection [FD%d]", fd);
        const esw::OneToAll &oneToAll = request.onetoall();
        uint64_t val = grid.processOneToAll(oneToAll);

        processLogger.info("OneToAll response %llu on connection [FD%d]", val, fd);
        response.set_total_length(val);

    } else if (request.has_reset()) {
        processLogger.warn("Reset message received on connection [FD%d]", fd);
        const esw::Reset &reset = request.reset();
        grid.processReset(reset);

    } else {
        processLogger.error("No valid message type detected on connection [FD%d]", fd);
        response.set_status(esw::Response_Status_ERROR);
    }

    // Send the response
    writeResponse(response, fd);

    // Final request should close the connection
    if (request.has_onetoall()) {
        shutdown(fd, SHUT_RDWR);
        processLogger.info("Closing connection after OneToAll request on connection [FD%d]", fd);
    }
}

void EpollConnectEntry::writeResponse(esw::Response &response, int fd) {
    // Get the size of the serialized response
    size_t size = response.ByteSizeLong();

    processLogger.debug("Response size: %d status: %d on connection [FD%d]", size, response.status(), fd);

    // Convert the response size to network byte order
    int networkByteOrderSize = htonl(size);

    int bytesSent = send(fd, (const char *) (&networkByteOrderSize), 4, 0);
    if (bytesSent < 0) {
        processLogger.error("Failed to send response size on connection [FD%d]: %s", fd, string(strerror(errno)));
    } else {
        response.SerializeToFileDescriptor(fd);
        fsync(fd);
    }
}
