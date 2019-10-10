
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


--udp.dump("localhost", 50720, "sdsadsa")

include("intellicon_host/init.lua")

-- local a = udp.open(1337, false, function(data, ip)
	-- print("received")
	-- print(ip)
	-- print(data)
-- end)


-- local t = udp.list()
-- print("Active UDP Sockets:")
-- for k, v in pairs(t) do
	-- print(k .. " : " .. tostring(v) .. " ")
-- end