<h1 align="center">
  <br>
  <img src="https://raw.githubusercontent.com/Michal-Szczepaniak/nanofiles/master/icons/172x172/nanofiles.png" width="256px" alt="Nanofiles">
  <br>
  <br>
  Nanofiles
  <br>
  <br>
</h1>

Nanofiles a feature filled file manager application for SailfishOS.

## Features
- Preview JPEG, PNG and GIF files
- Install Android APK and Sailfish packages
- Select multiple files
- Cut, copy and paste files
- Rename files and folders
- Create new folders
- Delete files and folders
- Show hidden files
- Change permissions
- Share files
- Root mode

## About this fork

This is an unofficial fork of [Michal-Szczepaniak/nanofiles](https://github.com/Michal-Szczepaniak/nanofiles),
built and tested on Sailfish OS 5.x (aarch64). Changes compared to upstream:

- **No root mode.** The setuid-root binary is removed; Nanofiles runs as the normal user.
- **Image previews for all file sizes** via the system thumbnailer, shown uncropped.
- **Remote SFTP location (sshfs).** An optional pull-down menu entry *Connect / Disconnect / Open*
  mounts one remote host with sshfs. Configure it in `~/.config/nanofiles/sshfs.conf`
  (see the comment at the top of `/usr/bin/nanofiles-sshfs`). Key-based login only;
  the host key must already be in `~/.ssh/known_hosts`. Requires `fuse-sshfs` (e.g. from Chum).
- **Finnish translation.**
- **"Reset entries" can be undone** (remorse timer) instead of wiping custom places immediately.
- SortFilterProxyModel submodule patched to build with Qt 5.6
  ([jmpx448/SortFilterProxyModel](https://github.com/jmpx448/SortFilterProxyModel), branch `qt56`).

A prebuilt aarch64 RPM is available under [Releases](https://github.com/jmpx448/nanofiles/releases).
Install it with `devel-su pkcon install-local nanofiles-*.rpm`.
It is not signed and comes with no warranty.

## License
GNU General Public License v3.0