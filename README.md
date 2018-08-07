# Nimble Retroarchifcation

The following is a very simple tool that appends `--retroarch` to the
command line options of games for the NES Mini and SNES Mini for devices
modded with [Hakchi2](https://github.com/ClusterM/hakchi2) that contain
[retroarch-clover](https://github.com/ClusterM/retroarch-clover) app.

The Hakchi2 tool in no way provides a way to automate the configuration
of games to use retroarch, this does just that as the alternative is to
manually append `--retroarch` to each and every game individually.

This works for both Windows and Linux, the latter of which Hakchi2 does
not support, however a Hakchi2 modded console is discoverable on Linux
as a standard block device so you can add and patch games from a Linux
rig with this tool too.

The tool is safe to run multiple times (it won't patch games that have
already been patched.) So you can add and remove games without having to
start clean each time.
