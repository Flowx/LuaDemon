
_TEST = _TEST or 0
_TEST = _TEST + 1

include("duh/includeme.lua")
print("Run # " .. _TEST)
print("Root: " .. _LUAENV.ENVDIR)
print("Arch: " .. _LUAENV.ENVSYS)

local PORT_A = "ttyS560"



local data = {}

data[1] = 0x83
data[2] = 0x00
data[3] = 0xFF
data[4] = 0x53
data[5] = 0x00
data[6] = 0xFF
data[7] = 0x85
data[8] = 0x53

for k,v in pairs(data) do
	data[k] = string.char(v)
end



--local buff = table.concat(data)

net.dumpUDP("10.0.0.8", 64394, buff)

net.openUDP(1666, false, function(data, ip)
	print("call udp!")
	print("IP: " .. ip)
	print("Data: " .. data)
end)


-- local s = net.connect("10.0.0.8", 62770)
-- print(s)

-- local s = 0
-- s = net.openTCP(666, 5, function()
	-- print("call tcp!")
-- end)
-- print(s)















































-- net.openUDP(696, function()
	-- print("call 2!")
-- end)







