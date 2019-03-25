# C-Socket

## Summary
Repository for multiple small C-Socket project implementations. 

Contains:

- Simple HTTP Server
  - A Simple HTTP 1.0 server that responds with a file.
- Printpool
  - A Simple printerpool service. Clients have input for filename, they forward it to the server. Server queues them and sends them to printer when a printer is free.

## Installation & Usage

### Printpool

```console
$ make
$ ./client HOST:PORT
$ ./server LISTENPORT HOST1:PORT1 HOST2:PORT2 HOSTn:PORTn
$ ./printer LISTENPORT
```

Send filenames on client, watch them getting processed on printers.

#### Demonstration
[![asciicast](https://asciinema.org/a/213747.svg)](https://asciinema.org/a/213747)


### Simple HTTP Server

```console
$ make
$ ./client HOST:PORT
$ ./server LISTENPORT
```

Send request in format 'GET /\<filenaam\> HTTP/1.0' from client.

Also working:

```console
$ curl --http1.0 localhost:<listenport>/<filenaam>
```

#### Demonstration
[![asciicast](https://asciinema.org/a/213784.svg)](https://asciinema.org/a/213784)
---
For info Danielpot93@gmail.nl
