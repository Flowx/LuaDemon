
_TESTCYCLE = _TESTCYCLE or 0
_TESTCYCLE = _TESTCYCLE + 1

print("Run # " .. _TESTCYCLE)
print("Root: " .. _LUAENV.ENVDIR)
print("Arch: " .. _LUAENV.ENVSYS)

print("\n")

-- This overloads the default type() function
-- so it accept alternative names using __type
if not _G.__oldtype then -- just check if its already overloaded
	_G.__oldtype = _G.type
	_G.type = nil
	_G.type = function( v )
		local t = getmetatable(v)
		if t and t.__type then return t.__type end
		return _G.__oldtype(v)
	end
end



--local sock = tcp.open(1337, false, function(data) end)



local sock = tcp.open(1337, false, function() end)
print("Active Sockets: ")
	for k,v in pairs(tcp.list()) do
	print( k .. " : " .. tostring(v))
end

print("\nGetting Port:")
print(sock:getPort())

print("\nValid:")
print(sock:isValid())

print("\nClosing socket...")
sock:close()

print("\nValid:")
print(sock:isValid())

print("\nActive Sockets: ")
	for k,v in pairs(tcp.list()) do
	print( k .. " : " .. tostring(v))
end


print(sock)
--include("intellicon_host/init.lua")


