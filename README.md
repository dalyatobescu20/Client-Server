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

## SERVER

The server acts as a broker, facilitating message management within the platform. It opens two sockets (one TCP and one UDP) on a specified port and awaits connections/datagrams on all available local IP addresses.

### Startup

Start the server using the following command:

```bash
./server <PORT>
```

## Displayed Messages for Server

To monitor server activity, events such as client connections and disconnections are displayed as follows:

- **New client <ID_CLIENT> connected from IP:PORT.**
- **Client <ID_CLIENT> disconnected.**

# TCP Clients

TCP clients can subscribe and unsubscribe to topics by sending messages to the server.

## Startup

To start a TCP client, use the following command:

```bash
./subscriber <ID_CLIENT> <IP_SERVER> <PORT_SERVER>
```

## Displayed Messages for TCP clients

For each command received from the keyboard, the client displays a feedback line as follows:

- **Subscribed to topic.**
- **Unsubscribed from topic.**
  
These messages are displayed only after the commands have been sent to the server.

# Displaying Received Messages

For each message received from the server (i.e., data from a topic to which the client is subscribed), the client immediately displays a message in the following format:

```bash
IP_CLIENT_UDP>:<PORT_CLIENT_UDP> - <TOPIC> - <DATA_TYPE> - <MESSAGE_VALUE>
```




