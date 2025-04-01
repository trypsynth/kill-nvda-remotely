Kill NVDA Remotely
This is a simple set of Windows programs that lets you restart NVDA on a remote machine (typically because you've lost your NVDA Remote connection).

Usage over a standard network:
1. Download the latest release and unzip it to a folder of your choice.
2. Open config.ini in your favorite text editor, and set the IP address and port. The IP address should be the remote machine's address.
3. Open the port on your router if you want to control NVDA from outside your local network.
4. Run server.exe on the remote machine.
5. Run client.exe on your local machine whenever you need to restart NVDA on the remote machine.
6. To exit the server, simply click on its system tray icon.

Usage Over Tailscale:
1. Install Tailscale on Both Machines, and sign in and authenticate each device.
2. Get the Tailscale IP Address. On the remote machine, open a command prompt and run "tailscale ip -4" without the quotes, and note the displayed IP address. This will be used as the server IP in config.ini.
3. Open config.ini in your favorite text editor, and set the IP address to the Tailscale IP of the remote machine. Set the port to any unused port (e.g. 5000).
4. Start `server.exe`. It will now listen for connections on the Tailscale network.
5. Start `client.exe`. It will connect to the remote machine over Tailscale and restart NVDA.
