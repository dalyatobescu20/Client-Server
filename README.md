# TCP and UDP Pub-Sub System

## Objectives

The objectives of this project are:

- Understanding the mechanisms used for developing network applications using TCP and UDP.
- Multiplexing TCP and UDP connections.
- Defining and utilizing a data type for a predefined protocol over UDP.
- Developing a client-server application using sockets.

## General Description

To achieve the objectives, we'll implement a platform consisting of three components:

- The server (singleton) will facilitate communication between clients on the platform, enabling message publication and subscription.
- TCP clients will behave as follows: a TCP client connects to the server, can receive commands such as subscribe and unsubscribe from the keyboard (user interaction), and displays messages received from the server on the screen.
- UDP clients (already implemented by the responsible team) will publish messages to the proposed platform using a predefined protocol.

The desired functionality is for each TCP client to receive messages from the server that are published by UDP clients and refer to the topics they are subscribed to.

## Server

The server acts as a broker, facilitating message management within the platform. It opens two sockets (one TCP and one UDP) on a specified port and awaits connections/datagrams on all available local IP addresses.

### Startup

Start the server using the following command:

```bash
./server <PORT> 

# Displayed Messages

To monitor server activity, events such as client connections and disconnections are displayed as follows:

- **New client <ID_CLIENT> connected from IP:PORT.**
- **Client <ID_CLIENT> disconnected.**

The server will not display any other messages apart from those specified. Further details regarding this aspect are explained in more detail in section 6.
