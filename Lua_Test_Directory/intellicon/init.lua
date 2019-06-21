print("Initializing Intellicon!\n")


do -- load and parse config file
	local content = file.readAll("intellicon_host.cfg")
	if content then content = json.parse(content) end
	content = content or {}

	_G.config = {}
	_G.config.ip_port 			= content.ip_port or 13337 -- IP port to run on
	_G.config.ip_port_interface = content.ip_port_interface or 1337 -- port for the interface (Desktop/App) to connect to

	local jstr = json.stringify(config)
	file.writeAll("intellicon_host.cfg", jstr)
end

do -- Open UDP Socket
	_G._socket = net.openUDP(config.ip_port, false, function(data, ip)
		print("Received Data!")
	end)

	if _G._socket then -- check if the socket opened successfully
		print("Sucessfully opened UDP port on " .. config.ip_port) 
	else
		error("Something went wrong creating the socket!")
	end
end

do -- Open the main TCP Socket
	_G._socket = net.openTCP(config.ip_port_interface, function(data, ip)
		print("Received Data!")
	end)

	if _G._socket then -- check if the socket opened successfully
		print("Sucessfully opened TCP port on " .. config.ip_port_interface) 
	else
		error("Something went wrong creating the socket!")
	end
end

net.dumpUDP("10.0.0.255", config.ip_port, "$$$INTELLICON:DISCOVERY$$$")







