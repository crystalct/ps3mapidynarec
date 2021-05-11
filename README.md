# PS3 MAPI DYNAREC LIB
This is a very simple library (it's just `ps3mapidyn.h`) for write Dynarec Code on PS3, using PS3 MAPI.

## PS3 Manager API (PS3MAPI)

PS3 Manager API aka PS3M_API is an API similar to TMAPI or CCAPI, compatible with CEX and DEX consoles. It includes a lot of new features (see below) which can be used in any PS3 plugins (vsh plugin, game plugin, ...), homebrew and it can be used remotely with any PC tools (and Android, iOS, ... is open source so anyone can port the PC library to any other platform) or with the webUI (any device with an internet navigator).

Developer: _NzV_

You can use [PS3MAPI Library](https://github.com/bucanero/ps3mapi-lib) by @bucanero, just put `ps3mapi.h` before `ps3mapidyn.h`, add  `-lps3mapi` to `Makefile` and use all PS3MAPI features (just for PSL1GHT version).

Status:

PSL1GHT version: Working!\
Old SDK version: In testing....
