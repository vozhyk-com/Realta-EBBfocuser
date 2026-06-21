# Build guide

This is the full build guide for the Realta EBBfocuser.

## What you need to buy.

### Stepper motor

I have used a 23mm thick Nema 17 stepper motor like this 17HS4023. 

![Nema 17](../Guide/Images/17HS4023.png)

However, any NEMA stepper motor thicker than this will work but you are limited to 1.5A per phase by the EBB36 stepper motor driver. Even thinner motors will work but you will have a hard time mounting them as the case will foul most mounting brackets. The FreeCAD file containing the parts is included in this repository so can be edited to provide a spacer to pad out the difference in width if needed. 

### BigTreeTech EBB36 

There are two versions of this, one with an accelerometer and one without, both will work but the accelerometer version is twice as expensive and no use of it is made in this project. There are multiple versions of this PCB and this code will work with all of them however version 1 of the board uses a different pin for the optional dew heater control. Version 1.1 and 1.2 seem identical in function. 

![EBB36 Focuser PCB](../Guide/Images/EBB36PCB.png)

### Note about the EBB42

Originally this project was built around the EBB42 kit sold by big tree tech however we changed the design to use the EBB36 for two reasons; 

1) The PCB is smaller allowing for a smaller sized case and
2) The power connector while smaller is of a more common type so should be easier to source replacements for.

All of the software provided here will work just fine with the EBB42 but you will need to design your own case for it.

### M3 Hex bolts

To secure the EBB36 you will need 

4 25mm long M3 bolts to secure the top of the case

2 6mm M3 bolts to secure the PCB

2 to secure the stepper motor and their length depends on how thick the stepper motor is, the original bolts 
will not reach through the combined thickness of the case and motor so you will need 2 replacement ones. I found that for the 23mm thick motor I could use 22mm long M3 bolts, which is unfortunatly not that common a size but Amazon sellers did have them, the STL files for the printed parts offer a couple of options of different offsets for the mounting of these bolts which should allow for fine tuning to match bolt size to stepper motor thickness.

Please note I found it very easy to damage the screws these motors come with so please make sure you use an appropriately sized scew driver before attempting to remove them. 

### Optional: Crimping tools and ferrules

The EBB36 focuser comes with all the accessories you need to connect it up, it comes with all the cable fittings needed. However you will need to crimp these onto the wires yourself and a dupont or similar crimping tool greatly helps.

![Dupont crimping](../Guide/Images/DuPontCrimping.png)

However it is possible to crimp using Needle Nose Pliers. 

[![Needle Nose Pliers](http://img.youtube.com/vi/JsoqBS1-k7M/0.jpg)](http://www.youtube.com/watch?v=JsoqBS1-k7M "Needle Nose Pliers")

It can also be a good idea to use ferrules for terminating the dew heater wires used in the screw terminals.

![Dupont crimping](../Guide/Images/FerruleCrimping.png)

One kit will get you enough connectors to last a lifetime. 

### Power connector

The EBB36 comes with a power connector that needs to be crimped onto a power cable of somekind. Below are some examples.

![Connector Ideas](../Guide/Images/PowerConnectorIdeas.png)

I use the Anderson Power Pole connector in a victim cable configuration so that the Anderson connector receives the brunt of being plugged and unplugged as the connector on the PCB isn't really designed for that. I would however recommend cutting an existing cable for beginners because Anderson Power Poles are expensive and the DIY connectors sold on Amazon and the like need soldering and are fairly flimsy. 

## What you need to 3D print.

The parts are designed to be printed easily on a printer using a 0.8mm nozzle so should be easy prints on any printer using a smaller nozzle. 

![Printer](../Guide/Images/3DPrintBed.png)

The bottom section of the case needs supports to print well, I used the support paint tool and the smart fill option to select the faces needing supports using Prusa Slicer.

![Needs supports](../Guide/Images/NeedsSupports.png)

Please note the 4 screw holes needed to secure the case to the stepper motor will be bridged over during printing with a single layer of filament, this can be easily poked through with a small philips screw driver. 

The design should be printed at 0.2mm layer height with a 0.4mm nozzle. 

### Notes on filament type.

Stepper motors will get warm when operating and even when not moving they are being held in place by powered magnetic fields. They are designed to cope with very very high temperatures. The firmware provided does make use of the TMC2209's CoolStep technology which greatly lowers the current drawn when not moving however the motor will still get warm to the touch, around 40c to 50c is common. This means you really should think twice about using a filament such as PLA which will start going floppy around this temperature. PETG would be better but ABS or ASA would be best. 

## Assembling the case.

![Exploded](../Guide/Images/EBB36Exploded.png)

In this section we will cover the step by step assembly of the motor and case

### Shorten the stepper motor wires

The stepper motor you purchased probably came with a four wire cable with red, blue, green and black cables. However this cable will be far too long. It needs to be shortened to 70mm and this will require either cutting and resoldering the wires or recrimpling the connectors, most motors that are available on the webb will either have fixed wiring at the motor end or use a wider 6 pin connector, in the case of two connector motors measure from the larger connector as the EBB36 PCB kit will come with spare connectors and housings.

Stepper motors like these are designed to work with four wire two phase motors. The phases for the motor will be named A and B and a pair of wires will be used with each phase, the documentation for your stepper motor will sometimes tell you which colour wires go with each phase but there appears to be no standard colours for wiring so you should test the wires to see which go together if possible. To do this you can use a multimeter as matching pairs will show continuity.

[![EZTip #4 - Testing and Checking your Stepper Motors](http://img.youtube.com/vi/UI86W26lgl0/0.jpg)]([http://www.youtube.com/watch?v=JsoqBS1-k7M](https://www.youtube.com/watch?v=UI86W26lgl0) "EZTip #4 - Testing and Checking your Stepper Motors")

If you can't test the wires its not the end of the world if they get mixed up the motor will just jitter around and move ineffectively and you just need to swapped two of the wires around in the connector housing.

![Printer](../Guide/Images/MotorPinPhases.png)

The image above shows how the EBB36 PCB expects you to connect the phases, it doesn't matter that you connect AA from your motors documentation to AA on the header just that you keep both wires of each phase seperate from the others, swapping the order just reverses the direction of movement. The cables that came with my stepper motors had the pin order ABAB so I had to flip the two middle wires over.

### Remove two screws from the stepper motor

The case is mounted to the stepper motor using its own case mounting screws just using the longer replacement ones discussed earlier. The two screws that need removing are shown below. 

![Insert Nema 17](../Guide/Images/RemoveStepperScrews.png)

### Insert NEMA 17 stepper motor

The stepper motor will fit into the bottom half of the case when inserted at an angle.

![Insert Nema 17](../Guide/Images/InsertNema.png)

Make sure its wires are connected and to feed them through to the other side of the case.

Mount the case using the repacement bolts as shown below

![Screw Nema 17](../Guide/Images/screwNema.png)

Do not over tighten these (or any other bolts) just a moderate hand tighting will do.

### Postion Face Plate

The case face plate needs to be positioned over the USB-C port and power connector before fitting to the case.

![Postion Face Plate](../Guide/Images/FacePlate.png)

![Postion Face Plates](../Guide/Images/FacePlate2.png)

### Fit EBB36 PCB

![Fit PCB](../Guide/Images/ScrewPCB.png)

### Insert motor wire connector

The wire connector from the motor can now be connected to the PCB as shown below.

![Stepper motor connection](../Guide/Images/StepperMotorConnector.png)

### Fit top of case

Use two 10 mm hex bolts to screw the PCB into position. Do not over tighten.

![Fit PCB](../Guide/Images/ScewTopCase.png)

### Connect to your PC.

Connecting the EBB36 to your PC with a USB C cable will initially do nothing. The EBB36 is by default configured to use only power coming from its power connector, this power can range from a voltage of between 12v and 24v. So you need to connect both cables and make sure the power cable is powered. The default behaviour can be changed using one of the supplied jumpers however this is not recomended for this application.

![Attach wires](../Guide/Images/AttachWires.png)

## Building and flashing on Linux (arduino-cli + flash.sh)

If you are on Linux you can skip the Arduino IDE entirely and build/flash from the
command line. This is the workflow used by the `flash.sh` script in the root of this
repository, which compiles the sketch and uploads it over DFU in one step.

> **Note on protocol:** this fork's firmware speaks the **Moonlite focuser protocol**
> instead of the original custom G-code. On Linux that means you use the standard
> **INDI MoonLite focuser driver** (see the "Using the focuser with INDI" section
> below) rather than the Windows ASCOM driver. You can also test the board directly
> with the included `focuser_moonlite.py` script.

### 1. Install the prerequisites

You need four things on your `PATH`:

* **arduino-cli** — https://arduino.github.io/arduino-cli/ (tested with 1.5.1)
* **STM32CubeProgrammer** — https://www.st.com/en/development-tools/stm32cubeprog.html
  (provides `STM32_Programmer.sh`, used by the upload step)
* **dfu-util** and **usbutils** (`lsusb`) — from your distribution's package manager,
  e.g. `sudo apt install dfu-util usbutils`

STM32CubeProgrammer does not add itself to your `PATH`. Add its `bin` directory, for
example in `~/.bashrc`:

```bash
export PATH="$HOME/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin:$PATH"
```

Adjust the path to wherever you installed it, then open a new shell and confirm:

```bash
arduino-cli version
STM32_Programmer.sh --version
```

### 2. Install the STM32 board core and TMCStepper library

Tell arduino-cli where to find the stm32duino boards, then install the core and the
TMC2209 driver library:

```bash
arduino-cli config init
arduino-cli config add board_manager.additional_urls \
  https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
arduino-cli core update-index
arduino-cli core install STMicroelectronics:stm32
arduino-cli lib install TMCStepper
```

(The `SoftwareSerial` library used for TMC UART ships with the STM32 core — no separate
install needed.)

### 3. Enable EEPROM (FLASH_BANK_NUMBER edit)

This is the same flash-memory fix described in the Windows / Arduino IDE section below,
but the file lives in the arduino-cli data directory. Find the variant header (the core
version in the path will match what you installed):

```bash
find ~/.arduino15/packages/STMicroelectronics/hardware/stm32 \
  -name 'variant_EBB42_V1_1.h'
```

Open that file and make sure it contains the line:

```c
#define FLASH_BANK_NUMBER FLASH_BANK_1
```

Recent core versions already include it; older ones do not. **Without this line the code
compiles and uploads fine but the focuser never remembers its position.**

### 4. Allow non-root DFU access (one time)

In DFU mode the board shows up as USB ID `0483:df11`. To flash it without `sudo`, install
a udev rule (most STM32 setups already ship `45-stm32dfu.rules`):

```bash
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="df11", MODE="0666"' \
  | sudo tee /etc/udev/rules.d/45-stm32dfu.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

Once running, the focuser enumerates as a serial port (`/dev/ttyACM*`). Make sure your
user is in the `dialout` group to access it without `sudo`:

```bash
sudo usermod -aG dialout "$USER"   # log out and back in for this to take effect
```

### 5. Flash with flash.sh

Connect the EBB36/42 over USB **and** power it from 12–24 V, then run:

```bash
./flash.sh
```

The script compiles the sketch and then waits for the board to appear in DFU mode. Put
the board into upload mode — press and hold the button closest to the port panel, click
the other button once, then release both:

![click hold click](../Guide/Images/PutPCBintoUploadMode.png)

`flash.sh` detects the board (`0483:df11`), uploads the firmware, and reports when it is
done. Unplug and replug the board afterwards; it will reappear as `/dev/ttyACM*`.

> The board target is set in `flash.sh` via the `FQBN` variable
> (`pnum=EBB42_V1_1` — the EBB36 uses the same MCU and target). Edit that line if your
> hardware differs.

### 6. Test the focuser (optional)

With the firmware running you can drive the board directly using the included Python
client (needs `pyserial`: `pip install pyserial`):

```bash
./focuser_moonlite.py            # auto-detects /dev/ttyACM*
```

It opens an interactive prompt for moving, homing, reading temperature, and tuning the
StallGuard stall threshold. For production use, point your imaging software at the board
through the INDI MoonLite focuser driver instead.

## Setting up the Arduino IDE for use with EBB36.

### Install the Arduino IDE.

The latest Ardunio IDE can be found here.

https://www.arduino.cc/en/software

When you start the IDE for the first time you will be asked to allow it access to the internet, please allow it to do this as it will download various drivers and you will need this functionality later on to install some libaries. 

### Add additional Boards Managers URLs

The EBB36 doesn't use a microcontroller of a type used by any of the boards supplier by Arduino so we need to download additional board definitions. This can be done using the Arduino IDE itself. 

Go to file -> preferences and add stm32duino

![Board manager](../Guide/Images/ArduinoPreferences.png)

and add https://github.com/stm32duino/Arduino_Core_STM32/wiki/Getting-Started to the "additional Boards Managers" section at the bottom. Wait for the definitions to download

### Edit the configuration

As at the time of writing this guide (June 2024) the board definitions for the EBB36/42 do not allow you to use the micro controllers flash memory, not sure why this is as all thats needed is a simple configuration change. 

You need to use file explorer to go open a file in the following impossibly long directory

"%LocalAppData%\Arduino15\packages\STMicroelectronics\hardware\stm32\2.3.0\variants\STM32G0xx\G0B1C(B-C-E)(T-U)_G0C1C(C-E)(T-U)"

The file that needs editing is variant_EBB42_V1_1.h, its likely your PC doesn't have an application associated with this file type but its really just a text file so can be opened just fine in notepad.

![Board manager](../Guide/Images/ChooseAnotherApp.png)

![Board manager](../Guide/Images/NotePadSelect.png)

Once opened we need to add one line of text, #define FLASH_BANK_NUMBER	FLASH_BANK_1, as shown below.

![Board manager](../Guide/Images/EEPROM_WORK.png)

Please note that forgetting this step will lead to some confusion later on as the arduino code will compile and upload just fine but will not work correctly in practice as it will never remember its position.

### User board manager to add stm32duino

Back in arduino IDE use the "tools" -> Board manager menu at the top to add the STM32 boards to the IDE.

![Board manager](../Guide/Images/BoardsManager.png)

This opens a panel on the left hand side, in this search for "STM". 

![STM Search](../Guide/Images/STMSearch.png)

### Select and configure the board

Use the "tools" menu again to select "3D printer boards"; 

tools -> STM32 MCU based boards -> 3D printer boards

![Choose 3D Printers](../Guide/Images/3DPrinterBoards.png)

Then use the newly added menus to select the board part number, the Big Tree Tech EBB42 uses all the same hardware and connections just on a slightly larger PCB. 

![Choose EBB42](../Guide/Images/BoardPartNumber.png)

And upload method

![Choose Upload method](../Guide/Images/UploadMethod.png)

Finally the board is ready to be used in Arduino IDE!

### Install STM32CubeProgrammer

We still aren't done installing software, when the Arduino IDE uploads the file to the EBB42 board it will first try to compile it and when it does that it reaches out to the STM32 compiler which wont exist yet on your computer! In order for this to work we need to install STM32CubeProgrammer which can be downloaded from.

https://www.st.com/en/development-tools/stm32cubeprog.html

### Adding in required libraries.

In order to comunicate with the TMC2209 stepper driver on the EBB36 PCB we need computer code, we didn't write this fully ourselves we got some help and used a library that was written by teemuatlut, here's a link to his projects github https://github.com/teemuatlut/TMCStepper however you won't need to download anything from there as we can do that via the Arduino IDE itself. 

To do this we use the manage libraries function from the Sketch menu option

![Choose manage libraries](../Guide/Images/ManageLibraries.png)

Type TMCStepper into the search box and then install the latest version.

![Search for TMXStepper](../Guide/Images/TMCStepperLibrary.png)

The console in the bottom right will show success when installation is successful. 

![Console shows success](../Guide/Images/ConsoleSuccess.png)

Now arduino IDE is fully ready to be used to program the EBB36!

### Download and Unzip the Realta-EBBfocuser repository

Now we need to download the code and other files in this repository, on the main page there is a green "<> Code" button.

![GITHub download link](../Guide/Images/CodeDownloadZIP.png)

clicking it will give you the oprion to download a zip file, click that. This will result in a zip folder appearing in your downloads folder, right click on that an choose "Extract All..."

![Extract All](../Guide/Images/DownloadedZip1.png)

This will show a dialog box asking you where to unzip the file, leave the defaults but do click the "Shpw extracted files when complete" and click the "Extract" button.

![Save it where?](../Guide/Images/DownloadedZip2.png)

This should open a window showing a file structure similar to the image below.

![here are the folders](../Guide/Images/UnzippedFolder.png)

### Compiling and uploading source files using Ardunio IDE.

Go to the folder 

"%UserProfile%\Downloads\Realta-EBBfocuser-main\Realta-EBBfocuser-main\Arduino\EBBTelescopeFocuser" 

and double click "EBBTelescopeFocuser.ino"

![EBB42TelescopeFocuser.ino](../Guide/Images/OpenArduinoCodeFile.png) 

We are now ready to upload the arduino code to the EBB36 however we must first put the EBB36 into upload mode. Connect the EBB36 to your computer using a suitable cable and connect it to a 12v power source. You should here your computer beep when its connected. 

Once this id done the EBB36 can be put into upload mode by pressing two buttons on the PCB, you can do this either by removing the case or using two tooth picks and a little trial and error.

![click hold click](../Guide/Images/PutPCBintoUploadMode.png) 

You need to press and hold the button closest to the port panel and then click the other button once and release both. You should here a beep again as the device disconnects again.

Now that is done back in Arduino IDE we need to click the "upload" arrow button in the top left.

![upload arrow](../Guide/Images/uploadbutton.png)

Once clicked the arduino code will be first compiled and then uploaded to the focuser. The first time you do this it can take a while as the STM32CubeProgrammer from earlier is silently opened in the background. 

When the code is successfully uploaded you will get an output at the bottom of the arduino window something like below.

![success](../Guide/Images/uploadSuccessful.png)

You will hear another beep as the device reconnects to the PC. 

## Using the focuser with INDI

> The original project shipped a Windows **ASCOM** driver. Because this fork's firmware
> speaks the **Moonlite** protocol instead of the original custom G-code, that prebuilt
> driver no longer applies and has been removed. On Linux the focuser is driven through
> the standard **INDI MoonLite focuser driver** — no custom driver needed. (Any
> Moonlite-compatible client works, including N.I.N.A. on Windows via its built-in
> Moonlite driver.)

### Install INDI

Install INDI and a client such as KStars/Ekos from your distribution or
https://www.indilib.org/. On Debian/Ubuntu:

```bash
sudo apt install indi-bin kstars-bleeding   # or your distro's indi + kstars packages
```

The MoonLite focuser driver is part of the core INDI distribution, so no extra driver
package is required.

### Connect

1. Plug in the focuser (USB + 12–24 V power). It enumerates as `/dev/ttyACM0`
   (confirm with `ls /dev/ttyACM*`). Make sure your user is in the `dialout` group as
   described in the Linux build section above.
2. In Ekos, create a profile and add **MoonLite** as the Focuser.
3. Start the profile, open the focuser's **Connection** tab, set the **Port** to
   `/dev/ttyACM0`, and click **Connect**.
4. The focuser's current position and temperature now appear in Ekos, and you can run
   absolute/relative moves and autofocus.

### Settings

Unlike the old ASCOM driver, motor settings are stored in the board's EEPROM and tuned
with the included `focuser_moonlite.py` tool (see the "Test the focuser" step in the
Linux build section above) rather than from the client:

* **Motor current**, **microsteps**, **step delay** (speed) and **step mode**
  (full/half) are configured over serial and persisted on the board.
* **Stall detection** (TMC2209 StallGuard) can be enabled and tuned with the `sk`/`cr`
  commands.

Position, movement and temperature are then handled automatically by the INDI MoonLite
driver during normal imaging.
