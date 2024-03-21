using System.Net.Sockets;
using System.Net;
using System;

class Program
{
    static public bool IsRunning = true;

    static void Main()
    {
        while (IsRunning == true)
        {
            BackendServer.StartServer();
        }
    }
}

class BackendServer
{
    static int WebSocketPort;

    TcpListener Listener;

    public static void StartServer()
    {
        TcpListener Listener = new TcpListener(IPAddress.Parse("127.0.0.1"), WebSocketPort);
    }
}