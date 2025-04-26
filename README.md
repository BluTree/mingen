# mingen
**Min**imal project **gen**erator using lua scripts.

The goal of this is to create a simple environment to create projects, on multi-platforms, with the less friction possible by executing script directly instead of creating a declarative language over Lua.

Disclaimer: it is currently more a toy than a fully fledged project, used only by and for me in personal [mingen](https://github.com/BluTree/mincore) and [vulkanbox](https://github.com/BluTree/vulkanbox) projects.

## Functionalities

mingen generates [ninja build](https://ninja-build.org/) files, with clang as the main (and hardcoded) compiler/linker.

It works as a mono configuration, meaning a generation can only be made for one configuration. configuration can be specified in arguments (using `-c`, `--configuration` command line argument). An error will be printed if the configuration doesn't exists in the running file. Defaults to the first configuration provided by [`mg.configurations()`](/doc/api.md#mgconfigurations).

All the functions added in the Lua environment can be found [here](/doc//api.md)

## Todo
Many, many things need to be added/fixed to be used with all the features and stability I want:

- [ ] Add extensive test environment.
- [ ] Add missing features on linux.
- [ ] Supports other compilers/linkers, defined in scripts (for custom toolchains, shader compilation using Slang, etc...).
- [ ] Custom `require()` function to run another mingen file with a given configuration (for external projects) as well as path relative to the currently running script.
- [ ] Support for dynamic prebuilt libraries.
- [ ] Better error reporting.
- [ ] ...

## Building

### Windows
Run `windows.ninja` file. This will create a `bin/mingen.exe` file.

Currently tested environment is the following: clang 18+, with Windows 11 SDK (22621)

### Linux
Run `linux.ninja` file. This will create a `bin/mingen` file.

Currently tested environment is the following: clang 18+, with Fedora 38+

Warning: the linux version hasn't been tested since some times and will not compile due to the new features currently implemented in Windows only.
