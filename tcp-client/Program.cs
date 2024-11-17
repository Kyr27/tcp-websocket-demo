using WebSocketSharp;

internal class Program
{
    private static void Main(string[] args)
    {
        Console.WriteLine("C# - Client is active");

        // using here ensures that if anything goes wrong, it is properly disposed of and not left hanging
        using (WebSocket webSocket = new WebSocket("ws://localhost:8788"))
        {
            webSocket.OnMessage += WebSocket_OnMessage;

            webSocket.Connect();
            webSocket.Send("C# - Hello Server!");
        }

        Console.ReadKey();
    }

    private static void WebSocket_OnMessage(object? sender, MessageEventArgs e)
    {
        Console.WriteLine("C# - Received from server: " + e.Data);
    }
}