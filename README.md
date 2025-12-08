# PowerTunerDaemon

![](src/Resources/Windows/icon.ico)

PowerTuner Windows service / Linux daemon.

**Requires admin/root privileges**.

Runs a TCP server for clients to connect to, default port is **56000**.

Because it just waits for commands, CPU usage is 0% most of its lifetime.


Only one instance of the daemon is allowed on each device, and each daemon can be connected to only one client.

If a new client connection request is received, the previous client is disconnected.

It is possible to run multiple clients and connect each of them to a different daemon.

Start a client in any of your devices, or local device, and connect to the daemon:

CLI: [PowerTunerCLI](https://github.com/PowerTuner/PowerTunerCLI)

desktop: [PowerTunerClient](https://github.com/PowerTuner/PowerTunerClient)

console (gaming): [PowerTunerConsole](https://github.com/PowerTuner/PowerTunerConsole)

## Conflicts with similar applications

PowerTunerDaemon does **not** force settings, or other changes, to your device, under the hood.

This means that, more aggressive, or misbehaving, applications, may undo some of your changes soon after the daemon finishes to apply the settings.

To avoid issues, or undefined behavior, it is recommended to not run multiple tuning/tweaking applications at the same time.

The advise is valid for all OSes, including distro specific power daemons etc...

## Supported platforms

* Windows 10 22H2 or better
* Linux

PowerTunerDaemon is developed with multi-platform in mind, making it easy to add support for more platforms.

## Features

Most common and useful controls are implemented, for all supported OSes, where possible.

The following sections explain what features are implemented at the current stage.

## OS features

### Windows

Power schemes:

- fetching of all available settings, including hidden ones, with name and description
- duplicate power schemes (the only way to create new schemes)
- set active power scheme
- delete power schemes
- reset scheme to defaults
- replace default Windows schemes with custom ones

Fan control:

- GPD devices

### Linux

Sysfs:

- CPUs hotplug
- CPU frequency
- CPU scaling governor
- CPU idle governor
- Symmetric Multi Threading (SMT)
- Misc. power management (USB, I2C, block devices, ...)
- Block devices queue scheduler
- Intel GPU frequency
- AMD GPU power dpm force performance level
- AMD GPU legacy power dpm state
- AMD GPU frequency (SCLK)
- GPD fan control (requires [gpd_fan dkms](https://github.com/Cryolitia/gpd-fan-driver) or linux 6.18+)

## CPU features

### Intel

Supported CPUs:

```text
Clarkdale           [0x25]
Sandy Bridge        [0x2a]
Ivy Bridge          [0x3a]
Ice Lake Client     [0x7e]
Tiger Lake U        [0x8c]
Alder Lake N        [0xbe]
Lunar Lake          [0xbd]
```

_from [CPUModel.h](https://github.com/PowerTuner/PowerTunerDaemon/blob/main/src/Device/CPU/Intel/Include/CPUModel.h)_

- Power limits
- CPU power balance
- GPU power balance
- Undervolt (icelake and lower, ported from [intel-undervolt](https://github.com/kitsunyan/intel-undervolt))
- Turbo boost control
- HWP (_Hardware-Controlled Performance States_)
- More controls ([IntelCPU.cpp](https://github.com/PowerTuner/PowerTunerDaemon/blob/main/src/Device/CPU/Intel/IntelCPU.cpp))

### AMD

- RyzenAdj ([PowerTuner fork](https://github.com/PowerTuner/RyzenAdj))
- CPU P-State
- Performance boost control
- CPPC (_Collaborative Processor Performance Control_)

## Settings

```text
Boot profile:                   profile to apply on daemon start.
Battery profile:                profile to apply when running on battery.
Power supply profile:           profile to apply when charger is attached.
Address:                        TCP server listen address.
Apply interval:                 Re-apply last applied settings every X seconds.
Apply on wake from sleep:       Apply last applied settings on wake from sleep.
Ignore battery events:          Don't apply battery/power supply profiles.
Log level:                      Daemon logging level.
Max. log files:                 Maximum number of log files to keep on disk.
TCP port:                       TCP server port.
UDP port:                       [placeholder for future feature]
```

Change daemon settings from one of its clients.

## Logs

Logs are saved to:

```text
C:\ProgramData\PowerTunerDaemon
```

## winring0

winring0 driver is required in Windows for some features.

This driver is now actively blocked by Windows, and some modern anti-cheats, because it uses vulnerable functions.

PowerTunerDaemon loads/unloads the driver as needed, instead of keeping it loaded all the time.

No valid or working alternative exists at the current time, but, as soon as a valid replacement
is available, PowerTuner will switch to it.

When games with recent anti-cheats are running, all features that require winring will not be available,
due to winring driver being blocked by the game.

In that case, clients will log some errors, and all affected features will be greyed out.

After closing the game, reloading data from daemon, in client applications, is enough to re-enable all the features.

Help is appreciated to find or create/sign a replacement.

## Build

clone this repo:

```bash
git clone --recursive https://github.com/PowerTuner/PowerTunerDaemon.git
```

Build options:

```text
WITH_INTEL
enable support for Intel CPUs, default ON

WITH_AMD
enable support for AMD CPUs, default ON

WITH_GPD_FAN
enable support for GPD devices fan control, default ON

ENABLE_DBUS_SERVICES [linux only]
Enable support for wake from sleep and battery status change events on linux, default ON

WITH_SYSTEMD_NOTIFY [linux only]
Enable systemd notifications, required when running as systemd service, default ON
```

### Linux

requirements:

- gcc or clang
- cmake
- qt6-base
- kmod (**libkmod-dev** for debian based distros, or **kmod** for ArchLinux)
- libpci (WITH_AMD=ON, **libpci-dev** for debian based distros, or **pciutils** for ArchLinux)
- systemd-libs (WITH_SYSTEMD_NOTIFY=ON, **libsystemd-dev** for debian based distros, or **systemd-libs** for ArchLinux)

```text
$ cd PowerTunerDaemon
$ cmake -B build -DCMAKE_BUILD_TYPE=Release
$ make -C build
```

### Windows

requirements:

- Visual Studio 2022 with the component **Desktop development with C++** installed
- [Windows terminal](https://apps.microsoft.com/detail/9n0dx20hk701)
- [latest Qt](https://www.qt.io/development/download-qt-installer-oss) with **MSVC 2022 64bit** installed

Open terminal and create a new instance of **Developer Command Promp for VS 2022** or **Developer PowerShell for VS 2022**.

_Tab completition feature is only available in PowerShell_

```text
$ cd PowerTunerDaemon
$ cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="<path to Qt>\msvc2022_64\lib\cmake" -G "Ninja"
$ ninja -C build
```

Replace _\<path to Qt\>_ with your Qt root path, default is **C:\Qt\\<Qt version\>**

---

_Windows help output_
```
Options:
  -?, -h, --help  Displays help on commandline options.
  --help-all      Displays help, including generic Qt options.
  -a <address>    listen on address|localhost|any, default any
  -p <port>       port, default 56000
  --nc            disable client connection, no TCP/UDP server
  --installsvc    Install PowerTunerDaemon service
  --uninstallsvc  Uninstall PowerTunerDaemon service
  --startsvc      Start PowerTunerDaemon service
  --stopsvc       Stop PowerTunerDaemon service
  --nosvc         Run in portable mode instead of a service
```
