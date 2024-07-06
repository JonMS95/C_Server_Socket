# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.3 (Unreleased)]
### Added
- API header file now includes some more helpful functions to be applied once the socket has started running. The full API functions list goes like this:
  - ServerSocketGetClientIPv4
  - ServerSocketRead
  - ServerSocketWrite
  - ServerSocketRun

### Changed
- API function allows more input parameters, such as timeouts or setting the socket as non-blocking.

### Fixed
- Fixed some issues related to security that prevented the server from properly connecting to the client securely while security (TLS) was enabled.


## [1.2] - 2023-11-24
### Changed
- API function allows a more generic interaction function pointer.

### Fixed
- Added visibility modifiers in order to avoid potential naming conflicts in the future.

## [1.1] - 2023-11-18
### Added
- TLS (Transport Layer Security).
- Multiple connections handling.
- Custom dependency management tool for owner repos ([C_Common_shell_files](https://github.com/JonMS95/C_Common_shell_files)).

### Changed
- Modified API function (ServerSocketRun) by making it possible to add a custom interaction function.


## [1.0] - 2023-10-15
### Added
- First version of the library
