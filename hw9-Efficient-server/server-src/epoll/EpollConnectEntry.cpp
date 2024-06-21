
#include "EpollConnectEntry.hh"

// Global variables -------------------------------------------------------------------------------
PrefixedLogger connectLogger = PrefixedLogger("[CONNECTION]", true);
PrefixedLogger processLogger = PrefixedLogger("[PROCESSING]", true);

// Class definition -------------------------------------------------------------------------------
bool EpollConnectEntry::handleEvent(uint32_t events) {
    if (!this->is_fd_valid()) {
        connectLogger.error("Invalid file descriptor FD%d", this->get_fd());
        return false;
    }
    else if (events & EPOLLERR) {
        // Retrieve the specific error code
        int err = 0;
        socklen_t len = sizeof(err);
        if (getsockopt(this->get_fd(), SOL_SOCKET, SO_ERROR, &err, &len) == 0) {
            connectLogger.error("EPOLLERR received on connection FD%d: %s", this->get_fd(), strerror(err));
        } else {
            connectLogger.error("EPOLLERR received on connection FD%d: Unable to retrieve error code", this->get_fd());
        }
        return false;
    }
    else if (events & EPOLLHUP) {
        connectLogger.warn("EPOLLHUP received on connection FD%d", this->get_fd());
        return false;
    }
    else if (events & EPOLLRDHUP) {
        connectLogger.error("EPOLLRDHUP received on connection FD%d", this->get_fd());
        return false;
    }
    else if (events & EPOLLIN) {
        try {
            readEvent();
        }
        catch (exception &e) {
            connectLogger.error("readEvent(): %s on connection FD%d", e.what(), this->get_fd());
            return false;
        }
    }

    return true; // Keep the connection going
}

void EpollConnectEntry::readEvent() {
    if (processingInProgress) {
        return;
    }

    // Read the message size
    if (!messageInProgress) {
        // New message
        inProgressMessageSize = readMessageSize();
        if (inProgressMessageSize < 0) {
            connectLogger.error("Failed to read message size on connection FD%d", this->get_fd());
            shutdown(this->get_fd(), SHUT_RDWR);
            return;
        }

        inProgressMessageRead = 0;
    }

    // Message in progress
    int received = recv(this->get_fd(), messageBuffer + inProgressMessageRead,
                        inProgressMessageSize - inProgressMessageRead, MSG_WAITALL);

    if (received == 0) {
        connectLogger.debug("Connection closed by client on FD%d", this->get_fd());
        shutdown(this->get_fd(), SHUT_RDWR);
        return;
    }

    if (received < 0) {
        connectLogger.error("Failed to read message: %s on FD%d", string(strerror(errno)), this->get_fd());
        shutdown(this->get_fd(), SHUT_RDWR);
        return;
    }

    inProgressMessageRead += received;
    connectLogger.debug("Received: %d on connection FD%d", received, this->get_fd());

    // Was everything read from the socket?
    if (inProgressMessageRead != inProgressMessageSize) {
        connectLogger.debug("Message in progress (received %d out of %d) on connection FD%d", inProgressMessageRead,
                            inProgressMessageSize, this->get_fd());
        messageInProgress = true;
        return;
    }

    // All the message was read
    esw::Request request;
    esw::Response response;
    response.set_status(esw::Response_Status_OK);
    request.ParseFromArray(messageBuffer, inProgressMessageSize);

    connectLogger.info("Message handed to processing on connection FD%d", this->get_fd());
    processingInProgress = true;
    resourcePool.run([this, request, response] {
        processMessage(request, response);
        this->processingInProgress = false;
    }, this->get_fd());

    // Clean up after successful message processing
    messageInProgress = false;
}

int EpollConnectEntry::readMessageSize() {
    int msgSize;
    char sizeBytes[4];
    int received = recv(this->get_fd(), sizeBytes, sizeof(sizeBytes), MSG_WAITALL);

    if (received == 0) {
        connectLogger.debug("Connection closed by client on FD%d", this->get_fd());
        return -1; // signifying that the connection should be closed
    }

    if (received != 4) {
        connectLogger.error("On size read (expected 4) got: %d on connection FD%d", received, this->get_fd());
        esw::Response response;
        response.set_status(esw::Response_Status_OK);
        writeResponse(response);
        return -1; // signifying that the connection should be closed
    }

    memcpy(&msgSize, sizeBytes, sizeof(int));
    // Converts u_long from TCP/IP network order to host byte order
    msgSize = ntohl(msgSize);
    connectLogger.debug("Expecting: %d on connection FD%d", msgSize, this->get_fd());
    return msgSize;
}

void EpollConnectEntry::processMessage(esw::Request request, esw::Response response) {
    // Parse the message request
    if (request.has_walk()) {
        // The message is of type Walk
        processLogger.warn("Walk message received on connection FD%d", this->get_fd());
        const esw::Walk &walk = request.walk();
        // Process the Walk message accordingly
        grid.processWalk(walk);

    } else if (request.has_onetoone()) {
        // The message is of type OneToOne
        processLogger.warn("OneToOne message received on connection FD%d", this->get_fd());
        const esw::OneToOne &oneToOne = request.onetoone();
        // Process the OneToOne message accordingly
        uint64_t val = grid.processOneToOne(oneToOne);

        processLogger.info("OneToOne response %llu on connection FD%d", val);
        response.set_shortest_path_length(val);

    } else if (request.has_onetoall()) {
        // The message is of type OneToAll
        processLogger.warn("OneToAll message received on connection FD%d", this->get_fd());
        const esw::OneToAll &oneToAll = request.onetoall();
        // Process the OneToAll message accordingly
        uint64_t val = grid.processOneToAll(oneToAll);

        processLogger.info("OneToAll response %llu on connection FD%d", val, this->get_fd());
        response.set_total_length(val);

    } else if (request.has_reset()) {
        // The message is of type Reset
        processLogger.warn("Reset message received on connection FD%d", this->get_fd());
        const esw::Reset &reset = request.reset();
        // Process the Reset message accordingly
        grid.processReset(reset);

    } else {
        processLogger.error("No valid message type detected on connection FD%d", this->get_fd());
        response.set_status(esw::Response_Status_ERROR);
    }

    // Send the response
    writeResponse(response);

    // Final request should close the connection
    if (request.has_onetoall()) {
        shutdown(this->get_fd(), SHUT_RDWR);
        processLogger.info("Closing connection after OneToAll request on connection FD%d", this->get_fd());
    }
}

void EpollConnectEntry::writeResponse(esw::Response &response) {
    // Get the size of the serialized response
    size_t size = response.ByteSizeLong();

    processLogger.debug("Response size: %d status: %d on connection FD%d", size, response.status(), this->get_fd());

    // Convert the response size to network byte order
    int networkByteOrderSize = htonl(size);

    int bytesSent = send(this->get_fd(), (const char *) (&networkByteOrderSize), 4, 0);
    if (bytesSent < 0) {
        throw runtime_error("[CONNECTION " + to_string(this->get_fd()) + "][ERROR]  Failed to send response size: " +
                            string(strerror(errno)));
    }

    response.SerializeToFileDescriptor(this->get_fd());
    fsync(this->get_fd());
}
