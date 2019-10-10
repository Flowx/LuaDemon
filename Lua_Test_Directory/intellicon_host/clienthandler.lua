


local socket_id = net.openTCP(config.ip_port, function(data, remote_ip)
	print("Data A from:", remote_ip)
	print("Data: ", data)
end)
