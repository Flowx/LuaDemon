




code_dir  = LuaDemon

#x64 / x86 / ARM
code_arch = x64

#compile everything and output an executeable
all:
	g++ -w -std=c++11  -pthread -L. $(code_dir)/Lua/$(code_arch)/libluajit.so $(code_dir)/*.h $(code_dir)/*.cpp $(code_dir)/Lua/*.h $(code_dir)/Lua/*.hpp -o /home/flowx/LuaTest/LuaDemon.o 
