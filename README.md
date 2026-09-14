# AisLive_pi
AisLive_pi is an OpenCPN plugin that connects to the OpenWaters.io AIS stream and fetches live AIS targets over the internet for display in OpenCPN"


## Documentation
Available in the online [User Manual](https://github.com/Kupofty/AisLive_pi/blob/main/manual/modules/ROOT/pages/index.adoc).


## Installation
Install directly through plugin catalogue (**OpenCPN → Options → Plugins**), or download packages from [Cloudsmith](https://cloudsmith.io/~kupoftyopencpn/repos/).

To build from source, see [`INSTALL.md`](INSTALL.md).


## Windows temporary workaround
> **Temporary fix** — This is required for the current Windows release.

On Windows installations, OpenCPN may fail to load the plugin because of a conflict with `msvcp140.dll`.

1. Close OpenCPN.
2. Open the OpenCPN installation directory (typically `C:\Program Files\OpenCPN`).
3. Locate the `msvcp140.dll` file.
4. Rename it to `msvcp140.bak`.
5. Start OpenCPN again.
