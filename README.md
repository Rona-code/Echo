# Echo

**Echo** is a lightweight, local-first desktop companion for Windows and Linux.

A small character lives on your screen, walks around, falls asleep when you're idle, reacts to your clicks, and can remind you of your events. Fully customizable — add your own companions directly from the application.

---

## Features

- **Desktop companion** that stays on top of your windows
- **Physics & behavior**
  - Walks around the screen
  - Collides with other windows (only Windows or Linux X11)
  - Falls asleep after inactivity
  - Bounces when clicked or when a reminder triggers
- **Custom pets**
  - Default fox included
  - Add your own companions directly from the application
- **Achievements system**
  - Multiple unlockable achievements
  - Enjoy to discover
- **Calendar & reminders**
  - Add events
  - Your companion notifies you
- **Multi-language**
  - French, English, Japanese
- **Completely local**
  - All data stored on your machine
  - No network requests

---

## Screenshots

<p align="center">
  <img src="https://github.com/user-attachments/assets/73892a81-f7f5-4872-8812-fa25466ec493" width="270" alt="Echo companion">
  &nbsp;&nbsp;&nbsp;
  <img src="https://github.com/user-attachments/assets/3705a3b0-6be7-4ea7-8aef-588bff0c5a42" width="300" alt="Echo achievements">
</p>

---

## Custom Pets

You can add your own companions directly from the application.

Just provide the required animation frames (idle, walk, sleep) through the built-in interface.

---

## Building

### Requirements

- CMake ≥ 3.16
- Qt 6 (Widgets)
- C++17 compiler
- Windows or Linux

### Build

```bash
# Using the provided script (recommended) to be modified for your environment.
./Build.ps1

# Or manually
mkdir build && cd build
cmake ..
cmake --build .
```
---

### Author

Made with care by Rona.
