




code_dir  = LuaDemon

#x64 / x86 / ARM
code_arch = x64

#compile everything and output an executeable
all:
	g++ -w -std=c++17 -pthread $(code_dir)/*.h $(code_dir)/*.cpp $(code_dir)/Lua/*.h $(code_dir)/Lua/*.hpp -o LuaDemon.o -L. $(code_dir)/Lua/Linux_$(code_arch)/libluajit.so

run:
	g++ -w -std=c++17 -pthread $(code_dir)/*.h $(code_dir)/*.cpp $(code_dir)/Lua/*.h $(code_dir)/Lua/*.hpp -o LuaDemon.o -L. $(code_dir)/Lua/Linux_$(code_arch)/libluajit.so
	./LuaDemon.o -dir Lua_Test_Directory/ -filespyinterval 500
