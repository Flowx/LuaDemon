#renames and places the Lua binary in the correct directory

CODE_DIR="LuaDemon"

#x64 / x86 / ARM
CODE_ARCH="x64"

cp $CODE_DIR/Lua/Linux_$CODE_ARCH/libluajit.so $CODE_DIR/Lua/Linux_$CODE_ARCH/libluajit-5.1.so.2
cp $CODE_DIR/Lua/Linux_$CODE_ARCH/libluajit-5.1.so.2 /usr/lib/