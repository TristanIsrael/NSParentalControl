/*
 * NS Parental Control
 * Copyright (C) 2025 Tristan Israël
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Server.hpp"
#include "../logger.h"

// This was heavily inspired by sys-clk's ipc server, a big thanks to those
// who wrote the original C version:
// --------------------------------------------------------------------------
// "THE BEER-WARE LICENSE" (Revision 42):
// <p-sam@d3vs.net>, <natinusala@gmail.com>, <m4x@m4xw.net>
// wrote this file. As long as you retain this notice you can do whatever you
// want with this stuff. If you meet any of us some day, and you think this
// stuff is worth it, you can buy us a beer in return.  - The sys-clk authors
// --------------------------------------------------------------------------
namespace Ipc {
    using namespace alefbet::pctrl::logger;

    constexpr uint64_t waitTimeout = UINT64_MAX;                // Wait timeout when processing

    /*! @brief Constructs a new IPC Server with the given name and max clients.
     * @param name The service name to register.
     * @param maxClients The maximum number of concurrent client sessions. */
    Server::Server(const std::string & name, const size_t maxClients) {
        // Set status variables
        this->error_ = false;
        this->handler = nullptr;
        this->maxHandles = maxClients + 1;
        this->handles.reserve(this->maxHandles);

        // Exit if invalid session count given
        if (maxClients < 1 || maxClients > MAX_WAIT_OBJECTS - 1) {
            logError("[IPC] Invalid number of sessions requested\n");
            this->error_ = true;
            return;
        }

        // Create server
        Handle serverHandle;
        this->serverName = smEncodeName(name.c_str());
        smInitialize();
        ::Result rc = smRegisterService(&serverHandle, this->serverName, false, maxClients);
        if (R_FAILED(rc)) {
            logError("[IPC] Couldn't create server: %i:%i\n", R_MODULE(rc), R_DESCRIPTION(rc));
            return;
        }

        logInfo("[IPC] Server started\n");
        this->handles.push_back(serverHandle);
    }

    /*! @brief Processes a session request from a specific client index.
     * @param index The index of the client handle to process.
     * @return true if the session was handled successfully (or closed), false on error. */
    bool Server::processSession(const int32_t index) {
        int tmp;

        // Wait for request
        ::Result rc = svcReplyAndReceive(&tmp, &this->handles[index], 1, 0, UINT64_MAX);
        if (R_FAILED(rc)) {
            logError("[IPC] Couldn't receive request (closing handle): %i\n", rc);
            svcCloseHandle(this->handles[index]);
            this->handles.erase(this->handles.begin() + index);
            return true;        // Return true as closing a session is valid behaviour
        }

        // Create object from received data
        Request * request = Request::fromTLS();
        if (!request) {
            logError("[IPC] An error occurred creating the request object (most likely bad header magic)\n");
            return false;
        }

        // Take action based on request type
        bool closeSession = false;
        switch (request->type()) {
            // Call handler to prepare response
            case Request::Type::Request: {
                uint32_t result = this->handler(request);
                request->setResult(result);
                request->toResponseTLS();
                break;
            }

            // Prepare default response
            case Request::Type::Close:
                request->setResult(0);
                request->toResponseTLS();
                closeSession = true;
                break;

            // Otherwise prepare error response
            default:
                logError("[IPC] Received unexpected CmifCommand (%i)\n", request->type());
                request->setResult(MAKERESULT(11, 403));
                request->toResponseTLS();
                break;
        }

        // Send response and delete object
        rc = svcReplyAndReceive(&tmp, &this->handles[index], 0, this->handles[index], 0);
        if (rc == KERNELRESULT(TimedOut)) {
            rc = 0;
        }
        delete request;

        // Close session on error or close request
        if (R_FAILED(rc) || closeSession) {
            logError("Closing session %i due to error/request\n", index);
            svcCloseHandle(this->handles[index]);
            this->handles.erase(this->handles.begin() + index);
        }

        return (R_SUCCEEDED(rc));
    }

    /*! @brief Accepts a new client session and adds it to the handles list.
     * @return true if a new session was accepted, false otherwise. */
    bool Server::processNewSession() {
        Handle session;
        ::Result rc = svcAcceptSession(&session, this->handles[0]);
        if (R_SUCCEEDED(rc)) {
            // Check we have room
            if (this->handles.size() >= this->maxHandles) {
                logError("[IPC] Couldn't handle new session due to limit\n");
                svcCloseHandle(session);

            // Add session to vector
            } else {
                this->handles.push_back(session);
            }

            return true;
        }

        return false;
    }

    /*! @brief Sets the request handler function for this server.
     * @param f The handler function to use. */
    void Server::setRequestHandler(Handler f) {
        this->handler = f;
    }

    /*! @brief Processes one iteration of the event loop, waiting for and handling requests.
     * @return true if the server is still running normally, false on error. */
    bool Server::process() {
        if (this->error_) {
            return false;
        }

        // Wait for a client to send a request/message
        int32_t handleIndex;
        ::Result rc = svcWaitSynchronization(&handleIndex, &this->handles[0], this->handles.size(), waitTimeout);
        if (R_VALUE(rc) == KERNELRESULT(TimedOut)) {
            return !this->error_;
        }

        if (R_SUCCEEDED(rc)) {
            // Check we're within range
            if (handleIndex < 0 || static_cast<uint32_t>(handleIndex) >= this->handles.size()) {
                logError("[IPC] svcWaitSynchronization returned out of range index: %i\n", handleIndex);
                this->error_ = true;
                return false;
            }

            // If the index is not zero then we need to handle that client's request
            bool ok = true;
            if (handleIndex != 0) {
                ok = this->processSession(handleIndex);

            // Otherwise prepare for a new session
            } else {
                ok = this->processNewSession();
            }

            // Exit on an error
            if (!ok) {
                logError("[IPC] Failed to handle %s %i request\n", handleIndex == 0 ? "server" : "client ", handleIndex);
                this->error_ = true;
            }
        }

        return !this->error_;
    }

    /*! @brief Destructor for Server, closing all handles and unregistering the service. */
    Server::~Server() {
        // Close all client handles
        for (size_t i = 1; i < this->handles.size(); i++) {
            svcCloseHandle(this->handles[i]);
        }

        // Finally close server handle
        if (!this->handles.empty()) {
            svcCloseHandle(this->handles[0]);
            ::Result rc = smUnregisterService(this->serverName);
            if (R_FAILED(rc)) {
                logError("[IPC] Couldn't unregister server: %i\n", rc);
            }
        }

        smExit();
    }
}