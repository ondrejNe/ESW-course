#include "EpollModel.hh"

// #define DEBUG_CONNECTION

#define KEEP_CONNECTION true
#define CLOSE_CONNECTION false

EpollConnection::EpollConnection(int fd, Grid &grid, ThreadPool &resourcePool)
        : grid(grid),
          resourcePool(resourcePool),
          messageInProgress(false),
          messageBuffer(nullptr),
          inProgressMessageSize(0),
          inProgressMessageRead(0) {
    // Assign the file descriptor of the accepted connection
    this->set_fd(fd);
    this->set_events(EPOLLIN | EPOLLET | EPOLLHUP | EPOLLRDHUP | EPOLLONESHOT);
}

void EpollConnection::Cleanup() {
#ifdef DEBUG_CONNECTION
    cout << "[CONNECTION " << this->get_fd() << "]  Deallocating buffer" << endl;
#endif
    if (messageBuffer != nullptr) delete[] messageBuffer;
}

bool EpollConnection::handleEvent(uint32_t events) {
#ifdef DEBUG_CONNECTION
    cout << "[CONNECTION " << this->get_fd() << "]  Handling events num " << events << " --------- NEW EPOLL EVENT -----------" << endl;
#endif

    bool returnValue = CLOSE_CONNECTION;
    // Checking for errors or the connection being closed
    if (events & EPOLLERR) {
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "][ERROR]  EPOLLERR" << endl;
#endif
    } else if (events & EPOLLHUP) {
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "][ERROR]  EPOLLHUP" << endl;
#endif
    } else if (events & EPOLLRDHUP) {
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "][ERROR]  EPOLLRDHUP" << endl;
#endif
    } else if (events & EPOLLIN) {
        try {
            returnValue = readEvent();
        }
        catch (exception &e) {
#ifdef DEBUG_CONNECTION
            cout << "[CONNECTION " << this->get_fd() << "][ERROR]  readEvent(): " << e.what() << endl;
#endif
        }
    }

    if (returnValue == CLOSE_CONNECTION) Cleanup();
    return returnValue;
}

int EpollConnection::readMessageSize() {
    int msgSize;
    char sizeBytes[4];
    int received = recv(this->get_fd(), sizeBytes, sizeof(sizeBytes), MSG_WAITALL);

    if (received == 0) {
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "]  Closed from client" << endl;
#endif
        return -1; // signyfing that the connection should be closed
    }

    if (received != 4) {
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "][ERROR]  On size read (expected 4) got: " << received << endl;
#endif

        esw::Response response;
        response.set_status(esw::Response_Status_OK);
        writeResponse(response);
        return -1; // signifying that the connection should be closed
    }

    memcpy(&msgSize, sizeBytes, sizeof(int));
    // Converts u_long from TCP/IP network order to host byte order
    msgSize = ntohl(msgSize);
#ifdef DEBUG_CONNECTION
    cout << "[CONNECTION " << this->get_fd() << "]  Message size: " << msgSize << endl;
#endif
    return msgSize;
}

void EpollConnection::writeResponse(esw::Response &response) {
    // Get the size of the serialized response
    size_t size;
    size = response.ByteSizeLong();

#ifdef DEBUG_CONNECTION
    cout << "[CONNECTION " << this->get_fd() << "]  Response size: " << size << endl;
    cout << "[CONNECTION " << this->get_fd() << "]  Resposne status: " << response.status() << endl;
#endif

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

bool EpollConnection::readEvent() {
    // Read the message size
    if (!messageInProgress) {
        // New message
        inProgressMessageSize = readMessageSize();
        if (inProgressMessageSize < 0) {
            return CLOSE_CONNECTION;
        }

        inProgressMessageRead = 0;
        messageBuffer = new char[inProgressMessageSize];
    }

    // Message in progress
    int received = recv(this->get_fd(), messageBuffer + inProgressMessageRead,
                        inProgressMessageSize - inProgressMessageRead, MSG_WAITALL);\

    if (received == 0) {
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "][ERROR]  Connection closed by client" << endl;
#endif
        return CLOSE_CONNECTION;
    }

    if (received < 0) {
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "][ERROR]  Failed to read message: " << string(strerror(errno)) << endl;
#endif
        return CLOSE_CONNECTION;
    }

    inProgressMessageRead += received;
#ifdef DEBUG_CONNECTION
    cout << "[CONNECTION " << this->get_fd() << "]  Received: " << received << endl;
#endif

    // Was everything read from the socket?
    if (inProgressMessageRead != inProgressMessageSize) {
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "]  Message in progress (recieved " << inProgressMessageRead << " out of " << inProgressMessageSize << " )" << endl;
#endif
        messageInProgress = true;
        return KEEP_CONNECTION;
    }

    // All the message was read
    esw::Request request;
    esw::Response response;
    response.set_status(esw::Response_Status_OK);
    request.ParseFromArray(messageBuffer, inProgressMessageSize);

    // Parse the message request
    if (request.has_walk()) {
        // The message is of type Walk
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "]  Walk message" << endl;
#endif
        const esw::Walk &walk = request.walk();
        // Process the Walk message accordingly
        grid.processWalk(walk);

    } else if (request.has_onetoone()) {
        // The message is of type OneToOne
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "]  OneToOne message" << endl;
#endif
        const esw::OneToOne &oneToOne = request.onetoone();
        // Process the OneToOne message accordingly
        uint64_t val = grid.processOneToOne(oneToOne);

#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "]  OneToOne response " << val << endl;
#endif
        response.set_shortest_path_length(val);

    } else if (request.has_onetoall()) {
        // The message is of type OneToAll
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "]  OneToAll message" << endl;
#endif
        const esw::OneToAll &oneToAll = request.onetoall();
        // Process the OneToAll message accordingly
        uint64_t val = grid.processOneToAll(oneToAll);

#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "]  OneToAll response " << val << endl;
#endif
        response.set_total_length(val);


    } else if (request.has_reset()) {
        // The message is of type Reset
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "]  Reset message" << endl;
#endif
        const esw::Reset &reset = request.reset();
        // Process the Reset message accordingly
        grid.processReset(reset);

    } else {
        // None of the fields in the oneof msg are set
        // Handle the case where no valid message type is detected
#ifdef DEBUG_CONNECTION
        cout << "[CONNECTION " << this->get_fd() << "][ERROR]  No valid message type detected" << endl;
#endif
        response.set_status(esw::Response_Status_ERROR);
    }

    // Send the response
    writeResponse(response);

    // Clean up after successful message processing
    messageInProgress = false;
    inProgressMessageSize = 0;
    inProgressMessageRead = 0;
    delete[] messageBuffer;
    messageBuffer = nullptr;

    // Final request should close the connection
    if (request.has_onetoall()) return CLOSE_CONNECTION;
    return KEEP_CONNECTION;
}
