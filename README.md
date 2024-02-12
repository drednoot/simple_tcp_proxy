# Simple TCP Proxy

C++ application that can redirect TCP requests, having practically no dependencies (only some standard C/C++ stuff)

# Usage

```
proxy <proxy port> <server ip> <server port>
```
 * `proxy port` - the proxy will open on this port for others to connect
 * `server ip`, `server port` - server address for requests to redirect to

The proxy will also create log file `log` in the executable directory

# Build

## Requirements

 * linux (uses socket.h)
 * make
 * gcc
 * stdc++ (usually comes bundled with g++)

## build steps

```
cd src
make
```
