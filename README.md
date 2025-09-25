# wRCtrl (WebRelayConTRoLler)

## What it does

the program provides a unified interface to manipulate the status of several web relay arrays available
on the market. This particular implementation provides a CLI that enables:

- an interactive session, where the user can manipulate relay status for any number of times, unless
  a special command is entered;
- a non-interactive session that, uses a mnemonic code to command a single relay;

## How commands are dispatched

Currently, I've implemented only HTTP exchanges through TCP/IP but, I *would* like to add additional
means of communication.

### Supported hardware platforms

- [KMTronic W8CR](https://www.kmtronic.com/lan-ethernet-ip-8-channels-web-relay-board.html)
- NC800

This list may become larger in the future;

## Requirements

a C compilation system and curlib 7.81.0

## How to build it

I've provided a Makefile (usually, I prefer to use basic tools) to build the project. Open
a shell in the directory containing the src-* and headers-* sub-directories and simply run

> make

I've not built the executable on a Windows system (it requires an ad-hoc distribution like
Cygwin). The same holds true for Apple systems. You may try it but, I have no intention
of handing technical support for such endeavours;
the build process can be reversed by invoking

> make clean

the command will remove all files contained in the sub-directories bin and obj and the
sub-directories themselves

## How to run it

the command synopsis can be retrieved by invoking:

> ~/\<hier\>/bin/wRCtrl --help
> (\<hier\> is the parent directory of the project)

in general, three parameters need to be specified:
- the IPv4 address of the array;
- the model;
- the behaviour of the program;
additionally, if the model is the NC800, a port has to be included (it does not behave as a transport
layer SAP but, as a sort of hierarchical component within the URI);

> **interactive session**: *./wRCtrl --ipv4=\<ipv4\> --model=\<model\> --behaviour=iter [--port=\<port\>]*

> **non-interactive session**: *./wRCtrl --ipv4=\<ipv4\> --model=\<model\> --behaviour=single [--port=\<port\>] --mnemonic-code=\<code\>*

### Components

\<model\> => KMTronic\_wr | NC800

\<relay-ID\> => 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8

\<code\> => t\_on\_\<relay-ID\> | t\_off\_\<relay-ID\>

### Behaviour

when the interactive behaviour is chosen, the user can enter one of three commands:

- quit
- turn on \<relay-ID\>
- turn off \<relay-ID\>

the quit command simply exits the session. The other two commands depend upon the hardware configuration of the relay
they act on:

+ if the contact is normally open, turn on closes it while turn off opens it;
+ if the contact is normally close, the reverse is true;

the mnemonic code *t_on_\<relay-ID\>* is identical to *turn on \<relay_ID\>* while, *t_off_\<relay_ID\>* is identical to
*turn off \<relay_ID\>*

### Output

a list of of relays with an indication of the status for each one. The NC800 is a special case, as I have chosen to
print the list of either the first or the last four relays.

### Using a container environment

[Container environment](docs/CONTAINER_ENVIRONMENT.md)
