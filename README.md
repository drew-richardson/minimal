# Minimal

This is the groundwork for a C microservices library. The goal is to learn by relying only on platform libraries and support most operating systems and architectures. This means that 3rd party libraries are avoided even if they would reduce development time, provide more features, or offer higher quality.

Things I'm proud of
- the build system doesn't use autotools, but building on top of make and a POSIX shell it has many of autotools features (see [configure](configure) and [make/make.mk](make/make.mk))
- portability across multiple OSs, compilers, POSIX shells and POSIX makes
- target identification (see [config/identify.c](config/identify.c) and [the build/make/target.mk make rule](make/make.mk))
- minimizing the inclusion of platform headers by discovering struct sizes and alignment during configuration (see [src/dr_types_impl.h](src/dr_types_impl.h), [config/dr_types.c](config/dr_types.c), and [the build/include/dr_types.h make rule](make/make.mk)) which even works when cross compiling
- only depends on platform libraries (with build time dependencies on cygwin for Windows)
- [green threading](src/dr_task.c)

## Status

### Mostly Done

- build system
- TCP/IP
- context switching
- scheduling
- async networking
- getopt

### Incomplete

- logging
- named pipes
- timed sleeping
- collections
- vfs
- 9p server/client

### Planned

- service discovery
- printf

## Prerequisites

Ensure you're running an upstream-supported version of your operating system and install a compatible compiler. For Windows, cygwin is also required at build time.

Supported Operating Systems:
- Android
- FreeBSD
- Linux
- macOS
- Microsoft Windows
- NetBSD
- OpenBSD
- Solaris

Supported Architectures:
- arm
- arm64
- i686
- x86_64

Supported Compilers:
- clang
- gcc
- Sun Studio
- Visual Studio

Supported Make:
- BSD make
- GNU make
- Solaris make

## Compiling

Run the configure script followed by make.

```
$ ./configure
...
$ make
...
```

You can optionally pass additional options to configure like CC or CFLAGS. For example, to compile using Visual Studio for x64

```
$ ./configure CC=cl CCAS=ml64
...
$ make
...
```

## Running the tests

```
$ make check
OK
OK
OK
OK
OK
$
```

## Deployment

```
$ make install
  INSTALL  install
```

## Usage

Start the 9p server. Note that this server is read only, but other 9p compatible servers like [fossil](https://en.wikipedia.org/wiki/Fossil_(file_system)) should also work.

```
$ build/dist/9p_server -p 7000
1521919519.694033624 main(src/9p_server.c:794) : Listening for clients
```

Connect to the server using the 9p client

```
$ build/dist/9p_client -a localhost -p 7000 sh
/ $
```

The server will log that a client has connected

```
1521303113.105771996 server_func(src/9p_server.c:695) : Accepted client
```

The client command `help` lists the available commands

```
/ $ help
ls <directories>
cat <file>
write <data> <file>
rm <name>
rmdir <name>
stat <name>
create <name> <perm>
mkdir <name> <perm>
chmod <perm> <name>
sh
help
/ $
```

`ls` lists directory information

```
/ $ ls
20000000777 drewrichardson users Wed Nov 29 08:36:20 2017                    0 hello
/ $ ls hello
10000000666 drewrichardson users Wed Nov 29 08:36:20 2017                    0 world
/ $ cd hello
/hello $ ls
10000000666 drewrichardson users Wed Nov 29 08:36:20 2017                    0 world
/hello $
```

`cat` reads the contents of a file

```
/hello $ cat world
Hello world
/hello $
```

`write` writes to a file

```
/hello $ write 'hi server' world
/hello $
```

The provided server does not persist this information, but prints it on the server stdout. Other 9p compatible servers should persist the change.

```
'hi server'
```

`stat` displays additional details about a file or directory

```
/hello $ stat world
   0        0 40        0     55ad4c69f1a0 10000000666 Wed Nov 29 08:36:20 2017 Wed Nov 29 08:36:20 2017                    0 world drewrichardson users drewrichardson
/hello $ cd ..
/ $ stat hello
   0        0 80        0     55ad4c69f200 20000000777 Wed Nov 29 08:36:20 2017 Wed Nov 29 08:36:20 2017                    0 hello drewrichardson users drewrichardson
/ $
```

The other commands do not work with the provided server, but should work properly on other 9p compatible servers.

## License

This project is licensed under the GPL v2.0 License - see [COPYING](COPYING) for details

## Acknowledgments

Thanks to:
- [musl](https://www.musl-libc.org/) for [src/getopt.c](src/getopt.c)
- [Linux](https://www.kernel.org/) for [src/list.h](src/list.h)
- [boost.context](https://github.com/boostorg/context) for inspiring [dr_task](src/dr_task.c)
