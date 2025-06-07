# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0] 07-06-2025
### Added
* Thread usage: from now on, POSIX threads are going to be used to run multiple instances instead of processes.

### Changed
* New directory/file naming convention is used now.
* Updated dependencies to their latest available versions.
* Modified Makefile in such way that .so libraries can be linked in a given order now.

### Fixed
* Each server socket instance is closed after service to a given client ends (in prior versions, sockets were never closed, leading them to run indefinitely).
* Re-organized many submodules (instance manager, SSL consumer files).


## [1.3] 02-08-2024
### Added
* API header file now includes some more helpful functions to be applied once the socket has started running. The full API functions list goes like this:
  - ServerSocketGetClientIPv4
  - ServerSocketRead
  - ServerSocketWrite
  - ServerSocketRun
* Documentation (license, contribution rules, readme and changelog)

### Changed
- API function allows more input parameters, such as timeouts or setting the socket as non-blocking.

### Fixed
- Fixed some issues related to security that prevented the server from properly connecting to the client securely while security (TLS) was enabled.


## [1.2] - 24-11-2023
### Changed
- API function allows a more generic interaction function pointer.

### Fixed
- Added visibility modifiers in order to avoid potential naming conflicts in the future.

## [1.1] - 18-11-2023
### Added
- TLS (Transport Layer Security).
- Multiple connections handling.
- Custom dependency management tool for owner repos ([C_Common_shell_files](https://github.com/JonMS95/C_Common_shell_files)).

### Changed
- Modified API function (ServerSocketRun) by making it possible to add a custom interaction function.


## [1.0] - 15-10-2023
### Added
- First version of the library
