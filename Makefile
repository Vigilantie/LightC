#
#    LightC - the official compiler for the Light programming language
#
#    Copyright (C) 2026  Jacob T. Ward
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#

.PHONY: all install examples clean release rus-release

# Prefer gcc, fallback to clang
SRC = src/main.c src/lexer.c src/compile.c
PROJECT = lightc

all:
	mkdir -p build
	clang -O0 -march=native -flto $(SRC) -o build/$(PROJECT)

release:
	mkdir -p build
	clang -O2 -march=native -flto -DNDEBUG $(SRC) -o build/$(PROJECT)

rus-release:
	mkdir -p build
	clang -O2 -march=native -flto -DLANG_RU -DNDEBUG $(SRC) -o build/$(PROJECT)

install:
	cp build/$(PROJECT) /usr/local/bin/
	chmod +x /usr/local/bin/$(PROJECT)

examples:
	./build/$(PROJECT) examples/example.light

clean:
	rm -rf build
