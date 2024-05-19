
#include "EpollModel.hh"

bool EpollConnectEntry::handleEvent(uint32_t events) {
    connectLogger.debug("Handling events num %d", events);

    if (!this->is_fd_valid()) {
        connectLogger.error("Invalid file descriptor");
        return false;
    }
    else if (events & EPOLLERR) {
        connectLogger.error("EPOLLERR");
        return false;
    }
    else if (events & EPOLLHUP) {
        connectLogger.warn("EPOLLHUP");
        return false;
    }
    else if (events & EPOLLRDHUP) {
        connectLogger.error("EPOLLRDHUP");
        return false;
    }
    else if (events & EPOLLIN) {
        try {
            readEvent();
        }
        catch (exception &e) {
            connectLogger.error("readEvent(): %s", e.what());
            return false;
        }
    }

    return true; // Keep the connection going
}

void EpollConnectEntry::readEvent() {
    // Read the message size
    if (!messageInProgress) {
        // New message
        inProgressMessageSize = readMessageSize();
        if (inProgressMessageSize < 0) {
            shutdown(this->get_fd(), SHUT_RDWR);
            return;
        }

        inProgressMessageRead = 0;
        messageBuffer = new char[inProgressMessageSize];
    }

    // Message in progress
    int received = recv(this->get_fd(), messageBuffer + inProgressMessageRead,
                        inProgressMessageSize - inProgressMessageRead, MSG_WAITALL);

    if (received == 0) {
        connectLogger.debug("Connection closed by client");
        shutdown(this->get_fd(), SHUT_RDWR);
        return;
    }

    if (received < 0) {
        connectLogger.error("Failed to read message: %s", string(strerror(errno)));
        shutdown(this->get_fd(), SHUT_RDWR);
        return;
    }

    inProgressMessageRead += received;
    connectLogger.debug("Received: %d", received);

    // Was everything read from the socket?
    if (inProgressMessageRead != inProgressMessageSize) {
        connectLogger.debug("Message in progress (received %d out of %d)", inProgressMessageRead,
                            inProgressMessageSize);
        messageInProgress = true;
        return;
    }

    // All the message was read
    esw::Request request;
    esw::Response response;
    response.set_status(esw::Response_Status_OK);
    request.ParseFromArray(messageBuffer, inProgressMessageSize);

    resourcePool.run([this, request, response] {
        processMessage(request, response);
    });

    // Clean up after successful message processing
    messageInProgress = false;
    inProgressMessageSize = 0;
    inProgressMessageRead = 0;
    delete[] messageBuffer;
    messageBuffer = nullptr;
}

int EpollConnectEntry::readMessageSize() {
    int msgSize;
    char sizeBytes[4];
    int received = recv(this->get_fd(), sizeBytes, sizeof(sizeBytes), MSG_WAITALL);

    if (received == 0) {
        connectLogger.debug("Connection closed by client");
        return -1; // signifying that the connection should be closed
    }

    if (received != 4) {
        connectLogger.error("On size read (expected 4) got: %d", received);
        esw::Response response;
        response.set_status(esw::Response_Status_OK);
        writeResponse(response);
        return -1; // signifying that the connection should be closed
    }

    memcpy(&msgSize, sizeBytes, sizeof(int));
    // Converts u_long from TCP/IP network order to host byte order
    msgSize = ntohl(msgSize);
    connectLogger.debug("Message size: %d", msgSize);
    return msgSize;
}

void EpollConnectEntry::processMessage(esw::Request request, esw::Response response) {
    // Parse the message request
    if (request.has_walk()) {
        // The message is of type Walk
        connectLogger.info("Walk message received");
        const esw::Walk &walk = request.walk();
        // Process the Walk message accordingly
        grid.processWalk(walk);

    } else if (request.has_onetoone()) {
        // The message is of type OneToOne
        connectLogger.info("OneToOne message received");
        const esw::OneToOne &oneToOne = request.onetoone();
        // Process the OneToOne message accordingly
        uint64_t val = grid.processOneToOne(oneToOne);

        connectLogger.info("OneToOne response %llu", val);
        response.set_shortest_path_length(val);

    } else if (request.has_onetoall()) {
        // The message is of type OneToAll
        connectLogger.info("OneToAll message received");
        const esw::OneToAll &oneToAll = request.onetoall();
        // Process the OneToAll message accordingly
        uint64_t val = grid.processOneToAll(oneToAll);

        connectLogger.info("OneToAll response %llu", val);
        response.set_total_length(val);

    } else if (request.has_reset()) {
        // The message is of type Reset
        connectLogger.info("Reset message received");
        const esw::Reset &reset = request.reset();
        // Process the Reset message accordingly
        grid.processReset(reset);

    } else {
        connectLogger.error("No valid message type detected");
        response.set_status(esw::Response_Status_ERROR);
    }

    // Send the response
    writeResponse(response);

    // Final request should close the connection
    if (request.has_onetoall()) {
        shutdown(this->get_fd(), SHUT_RDWR);
        connectLogger.info("Closing connection after OneToAll request");
    }
}

void EpollConnectEntry::writeResponse(esw::Response &response) {
    // Get the size of the serialized response
    size_t size = response.ByteSizeLong();

    connectLogger.debug("Response size: %d", size);
    connectLogger.debug("Response status: %d", response.status());

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
