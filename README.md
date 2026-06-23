# Realta Scope Tech EBBfocuser

![EBB36 Focuser Case](Guide/Images/EBB36FinishedRCA.png)

Welcome to the Realta EBBfocuser Project! This initiative is dedicated to creating a precise, customizable, and open-source focuser for telescopes. By leveraging the power of the maker community and freely available hardware, we've developed a solution that promotes collaboration and innovation.

## Project Overview

The heart of our project is the Bigtree Tech EBB36, a versatile stepper motor driver and microcontroller. This component is combined with a 3D printed case and custom firmware developed using the Arduino IDE that speaks the Moonlite focuser protocol, so it works with the standard INDI MoonLite driver and any other Moonlite-compatible client. Together, these elements create a powerful and user-friendly telescope focuser designed for makers and enthusiasts who value customization and open-source principles.

## Key Components

**Bigtree Tech EBB36:** This stepper motor driver and microcontroller serves as the brain of our focuser, providing precise control over the telescope's focusing mechanism.

**3D Printed Case:** The case, designed to house the EBB36 and other components securely, can be easily printed using any standard 3D printer. It ensures the electronics are protected while maintaining accessibility for adjustments and upgrades.

**Arduino IDE Firmware:** Our custom firmware, developed in the Arduino IDE, allows for seamless communication between the EBB36 and the focusing mechanism. The firmware is open source, enabling users to modify and improve it according to their needs.

**Moonlite Protocol (INDI-compatible):** This fork's firmware implements the widely
supported Moonlite focuser protocol, so the focuser works with the standard INDI MoonLite
driver on Linux and any Moonlite-compatible client — no custom driver to install. (The
original project's Windows ASCOM driver targeted a different protocol and is not included
in this fork.)

## Features and Benefits

**Open Source:** All aspects of the Realta EBBfocuser, from the firmware to the case design, are open source. This allows users to freely modify, enhance, and share improvements, fostering a collaborative community.

**Maker-Centric Design:** The project is designed with makers in mind. Whether you enjoy 3D printing, coding, or electronics, the Realta EBBfocuser offers numerous opportunities to get hands-on and customize your setup.

**Precision:** The Bigtree Tech EBB36 offers high precision control, essential for achieving sharp focus in astrophotography and observation.

**Compatibility:** Speaking the Moonlite protocol, our focuser integrates with a wide array of existing astronomy software through the standard INDI MoonLite driver (and any other Moonlite-compatible client), enhancing its usability and versatility.
Getting Started

## To get started with the Realta EBBfocuser Project, you will need:

+ A Bigtree Tech EBB36 stepper motor driver and microcontroller
+ A 3D printer to print the custom-designed case
+ Basic electronics tools and components
+ The Arduino IDE (or `arduino-cli` on Linux — see the guide) for firmware installation and customization
+ A computer running INDI (or any Moonlite-compatible client) to control the focuser

We invite you to join our community, contribute to the project, and share your experiences. By working together, we can push the boundaries of what's possible in telescope focusing, making advanced astrophotography and observation more accessible to everyone.

The full guide to completing this project can be found here.

[Realta EBBfocuser complete guide](/Guide/ReadMe.md)

## Moonlite firmware & Linux build (this fork)

This fork replaces the original custom G-code serial protocol with the **Moonlite
focuser protocol**, so the focuser works out of the box with the standard
[INDI MoonLite driver](https://www.indilib.org/) on Linux (and any Moonlite-compatible
client) — no Windows or ASCOM required. The firmware also adds an optional NTC
temperature reading (TH0 header) and TMC2209 StallGuard-based stall detection.

Extra tooling included here:

+ **`flash.sh`** — compile and flash the firmware on Linux with `arduino-cli` +
  STM32CubeProgrammer (DFU upload), no Arduino IDE needed.
+ **`focuser_moonlite.py`** — interactive Python client for testing the Moonlite
  firmware over USB serial (requires `pyserial`).

See the [Linux build & flash section](/Guide/ReadMe.md#building-and-flashing-on-linux-arduino-cli--flashsh)
of the guide for setup steps, and [Using the focuser with INDI](/Guide/ReadMe.md#using-the-focuser-with-indi)
for connecting it to imaging software.

> **Note:** the original Windows ASCOM driver targeted the original G-code protocol and
> is **not** compatible with this fork's Moonlite firmware, so it has been removed.
> You can still build the firmware with the Windows Arduino IDE if you prefer (see the
> guide), but use a Moonlite-compatible client (INDI, or N.I.N.A.'s Moonlite driver) to
> control it.
