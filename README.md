# tcp-websocket-demo

This repository shows how to use websocket-sharp as a C# client, and websocketpp as a C++ server, all while (probably, not tested) staying cross-platform.
This method allows you to effectively communicate between two different applications that both use different languages.

## Dependencies
The C# side uses [websocket-sharp](https://github.com/sta/websocket-sharp), so you might need to install that (you can get it via nuget package manager)
The C++ side relies on [asio](https://think-async.com/Asio/) and [websocketpp](https://github.com/zaphoyd/websocketpp), both of which are already included in tcp-server/libs folder, so no need to install anything
