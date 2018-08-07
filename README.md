# Nimble Retroarchifcation

The following is a very simple tool that appends `--retroarch` to the
command line options of games for the NES Mini and SNES Mini for devices
modded with [Hackchi2](https://github.com/ClusterM/hakchi2) that contain
[retroarch-clover](https://github.com/ClusterM/retroarch-clover) app.

The Hackchi2 tool in no way provides a way to automate the configuration
of games to use retroarch, this does just that.

This works for both Windows and Linux, the latter of which Hackchi2 does
not support, however a Hackchi2 modded console is discoverable on Linux
as a standard block device so you can add and patch games from a Linux
rig with this tool too.

The tool is safe to run multiple times (it won't patch games that have
already been patched.) So you can add and remove games without having to
start clean each time.
