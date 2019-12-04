print("Initializing Intellicon Host!\n")

include("jsonparser.lua") -- stole this somewhere ...

do -- load and parse config file
	local content = file.readAll("intellicon_host.cfg")
	if content then content = json.parse(content) end
	content = content or {}

	_G.config = {}
	_G.config.ip_port 			= content.ip_port 			or 13337 -- IP port to run on
	_G.config.ip_port_interface = content.ip_port_interface or 1337 -- port for the interface (Desktop/App) to connect to

	local jstr = json.stringify(config)
	file.writeAll("intellicon_host.cfg", jstr)
end

_G.udpsocket = udp.open(config.ip_port, false, function(data, ip) 
	print("Received data from " .. ip)
end)

print("Active Sockets: ")
for k,v in pairs(udp.list()) do
	print( k .. " : " .. tostring(v))
end


-- Broadcast our existence
udp.dump("10.0.0.255", config.ip_port, "$$$INTELLICON:DISCOVERY$$$")





