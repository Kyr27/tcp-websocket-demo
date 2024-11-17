﻿using WebSocketSharp;

internal class Program
{
    private static void Main(string[] args)
    {
        // using here ensures that if anything goes wrong, it is properly disposed of and not left hanging
        using (WebSocket webSocket = new WebSocket("ws://localhost:9002"))
        {
            webSocket.OnMessage += WebSocket_OnMessage;

            webSocket.Connect();
            webSocket.Send("C# - Hello Server!");

            Console.WriteLine("C# - Client Online");
            Console.ReadKey(true);
        }
    }

    private static void WebSocket_OnMessage(object? sender, MessageEventArgs e)
    {
        Console.WriteLine("C# - Received from server: " + e.Data);
    }
}