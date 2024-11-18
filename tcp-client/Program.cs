using WebSocketSharp;

internal class Program
{
    private static void Main(string[] args)
    {
        // using here ensures that if anything goes wrong, it is properly disposed of and not left hanging
        using (WebSocket webSocket = new WebSocket("ws://localhost:9002"))
        {
            webSocket.OnMessage += WebSocket_OnMessage;
            webSocket.OnClose += WebSocket_OnClose;
            webSocket.OnError += WebSocket_OnError;

            webSocket.Connect();

            if (webSocket.IsAlive)
            {
                try
                {
                    webSocket.Send("C# - Hello Server!");
                }
                catch (Exception e)
                {
                    Console.WriteLine($"C# - Error while sending: {e.Message}");
                }
            }

            Console.WriteLine("C# - Client Online");
            Console.ReadKey(true);

            // Close the WebSocket explicitly, not necessary, but adds an extra layer of safety
            if (webSocket.ReadyState == WebSocketState.Open)
            {
                webSocket.Close(CloseStatusCode.Normal, "Client closing connection");
            }
        }
    }

    private static void WebSocket_OnError(object? sender, WebSocketSharp.ErrorEventArgs e)
    {
        Console.WriteLine($"C# - WebSocket error: {e.Message}");
    }

    private static void WebSocket_OnClose(object? sender, CloseEventArgs e)
    {
        Console.WriteLine($"C# - Server closed the connection: {e.Reason} (Code: {e.Code})");
    }

    private static void WebSocket_OnMessage(object? sender, MessageEventArgs e)
    {
        Console.WriteLine($"C# - Received from server: {e.Data}");
    }
}