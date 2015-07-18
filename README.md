LibrePCB Project Description
============================

[![Build Status](https://travis-ci.org/LibrePCB/LibrePCB.svg?branch=master)](https://travis-ci.org/LibrePCB/LibrePCB)

## About LibrePCB

LibrePCB is a free & OpenSource (GPLv3) Schematic / Layout Editor to develop printed circuit boards. The project is still in a quite early development status (no stable release available).

## Development Requirements

To compile LibrePCB, you need the following software components:
- g++ or MinGW >= 4.8 (C++11 support is required)
- Qt >= 5.4 (http://www.qt.io/download-open-source/)
- libglu1-mesa-dev (`sudo apt-get install libglu1-mesa-dev`)

## Workspace

At the first startup, LibrePCB asks for a workspace directory where the library elements and projects will be saved.
For developers there is a demo workspace inclusive library and projects in this repository (subdirectory "dev/workspace/").
