1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

The server first will send the length of the message, after the client recieves the length it starts reading exactly that many bytes from the message. The server also appends a special delimeter at the end of the message such as \n, \0 or EOF. In order to handle partial reads, a buffer can be used to store and process incomplete messages. Or looping and recv() calls until the expected number of bytes have been read.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

Since TCP is a byte stream protocol, a network shell protocol needs to be able to detect message boundaries. Some possible approaches are having fixed size messsages for each command, having the start of the message contian bytes that tell the amount of bytes the message includes, or each command is terminated by a unique character. Some challenges that could arise if this is not handled correctly, if multiple commands are sent back to back quickly, the client might read them as one single command without parsing it correctly. If the client is expecting more data to be sent based on incorrect prefix byte (determining the length of the string) it will hang and continue waiting until the proper amount of input is provided. 

3. Describe the general differences between stateful and stateless protocols.

Stateful protocols are able to maintain session information across multiple interactions. The server is able to remember past requests and the context relating to them. Some examples of this include HTTP (with cookies), however stateful protocols require more resources such as (memory and resource tracking). Stateless protocals treat each request independently (isolated bubble), with no memory of any previosu interactions. Some examples of this include REST API's and HTTP (without cookies); however, this requires resending the entire context every request. 

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

Despite UDB being unreliable, it has many advantages that can make it useful. It has low latency, efficient broadcasting and multicasting, lightweight protocol (reducing network congestion), and fast retries in networks with fast speeds (meaning that when packets are lost, they are retransmitted and dont wait TCP's congestion control). UDP is used a lot in video streaming and online gaming as they require efficent, lightweight, and low latency broadcasting and transmission. 

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

The operating system provides the Socket API as the primary interface for network communications. This allow complex network details to be simplified and allow applications to treat the network connections similar to files. The api includes functions like socket, bind, connect, send, listen, and recv. These handling the creation of connections and transferring of data. Sockets provide an easier and standard way for applications to communicate over networks.