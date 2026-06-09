# Light Compiler
## About
This is the compiler for the Light coding lanugage. this will take the Light language and convert it to C code, that will then compile to a native binary.

I have been learning Russian for a bit, and figured why not code in Russian too. So here is the compiler I made that I use to write some Russian code that will run natively.

## Features
The compiler supports basic variable declaration, console input and output. Along with any normal addition, subtraction multiplication, comparisons and stuff like that.

## Dependencies
You will need Clang
Thats it!

## Build
Alright, to the fun stuff, lets compile this baby.

From the project root, simply run:

```Bash
make release
make install
```
Or
```Bash
make rus-release
make install
```

for russian syntax build

That will compile and install it in the correct directory on your computer.
You can then run:

```Bash
make examples
```

This will compile all the Light examples

## Usage

To use this compiler, run

```Bash
lightc <file.light>
```

This will compile a Light file to C and then compile the C code to native binary.

## Light Coding Language
Checkout the [Learning](./docs/Learn.md) documentation to learn how to use this coding language.

## Licence
Licenced under GNU GPLv3, please refer to [LICENCE](./LICENCE.txt) for full licence.

All examples under the `./examples` directory are licenced under Public Domain, or MIT which ever you prefer.
