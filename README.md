![Imagination Technologies Limited logo](doc/images/img.png)

# LWM2M for Contiki

This repository contains LWM2M applications for Contiki

## Check Out Submodules

Ensure all git submodules are checked out with:

    $ git submodule init
    $ git submodule update

This should check out the contiki and LWM2M submodules into their relevant directories.

## Building the LWM2M Client for Contiki

### Contiki Simulated Device (x86)

Contiki has the ability to run as a simulated environment under Linux. This is useful for testing/development
purposes when hardware is not available.

    $ cd lwm2m-client-contiki
    $ make TARGET=minimal-net

This will build the executable lwm2m-client-contiki/lwm2m-client-contiki.minimal-net. See the section below in
regards to connecting this client to the gateway LWM2M server.

### Cross-Compiling Contiki

Building the Contiki image for a Seedeye or MikroE Clicker board currently requires Linux.

#### Microchip Toolchain Installation

Before targets can be compiled, the cross-compiler toolchain must be installed.

For 64-bit Ubuntu, 32-bit runtime libraries must be installed before the XC32 compiler can be run:

    $ sudo apt-get install libc6:i386

For 64bit Ubuntu the package lib32ncurses5 may also be required.

Installing the microchip XC32 compiler (~100 MB):

    $ wget http://ww1.microchip.com/downloads/en/DeviceDoc/xc32-v1.34-full-install-linux-installer.run
    $ chmod a+x xc32-v1.34-full-install-linux-installer.run
    $ sudo ./xc32-v1.34-full-install-linux-installer.run --mode unattended --Add\ to\ PATH 1 --debuglevel 4 --installerfunction installcompiler --netservername localhost
    # no output expected
    $ . ~/.bashrc
    $ xc32-gcc --version
    xc32-gcc (Microchip Technology) 4.5.2 MPLAB XC32 Compiler v1.34 Build date: Nov 21 2014
    Part support version: 1.34 (A)
    ...

There are reports that the installer does not modified PATH correctly. If trying to run xc32-gcc results in
"command not found", modify your .bashrc to set the path correctly:

    $ echo "export PATH=\$PATH:/opt/microchip/xc32/v1.34/bin" >> ~/.bashrc
    $ . ~/.bashrc

#### Compiling Contiki LWM2M client

Cross-compiling for the MikroE Clicker and Seedeye:

    $ cd lwm2m-client-contiki
    $ make TARGET=[target]

Where [target] is mikro-e for the MikroE Clicker and seedeye for the Seedeye.

This generates the HEX file `lwm2m-client-contiki.hex` for programming into a board using avrdude:

    $ make TARGET=[target] lwm2m-client-contiki.u

See "Programming a Mikro-e Clicker Board using AVRDude" for instructions on how to setup the MikroE Clicker with
an avrdude booatloader and how programming via avrdude is achieved. This is a WIP - hopefully the MikroE Clickers
will ship with the avrdude bootloader.

To generate a HEX file that can be used with the MPLAB IPE for programming via the ICD3 or any other programmer:

    $ cd lwm2m-cleint-contiki
    $ make TARGET=[target] BOOT=0

This generates the HEX file `lwm2m-client-contiki.hex` for programming into a board using MPLAB IPE, see the
next section for instructions.

#### Programming a Contiki HEX File into a MikroE Clicker Board using the ICD3

Programming the MikroE Clicker board currently requires MPLABX, an ICD3 Programmer and a Windows host.

1. Install Microchip MPLABX IDE for Windows version 3.05 from http://www.microchip.com/pagehandler/en-us/family/mplabx/home.html.
   Ensure that MPLAB IPE (Integrated Programming Environment) is installed. At the end of installation, you
   may be prompted to install a compiler - this is not required if using the Linux compiler above.

1. Ensure that 0-ohm resistors J3, J4 and J5 are installed on the PICkit side, not the mProg side. Desolder and move
   them if required.

1. Connect a USB-to-Serial adapter to the right-hand side of the Click interface by connecting:

        * Serial GND to Click GND
        * Serial RX to Click RF4/TX
        * Serial TX to Click RF5/RX

1. Connecting the USB-to-Serial adapter will create a COM port in Windows.
1. Start PuTTY and create a new connection on this COM port, with baud rate 115200, and no flow control.
1. Connect the ICD3 and ensure the driver has installed correctly. If the driver for "Microchip WinUSB Device" does not
   install correctly, use Device Manager to update the driver from this directory:

        C:/Program Files (x86)/Microchip/MPLABX/v3.05/Switcher/64Bit/winusb/amd64

1. Start the MPLAB IPE application.
1. Select the following Family and Device and click Apply:

        * Family: 32-bit MCUs (PIC32)
        * Device: PIC32MX470F512H

1. Click Settings and then Advanced Mode. Use the default password "microchip".
1. Click Power on the left side panel and then check the box ICSP Options > Power Target Circuit from Tool. Ensure the
   VDD is set to 3.3 volts.
1. Return to the Operate panel and click Connect:

        Connecting to MPLAB ICD 3...

        Currently loaded firmware on ICD 3
        Firmware Suite Version.....01.37.15 *
        Firmware type..............PIC32MZ

        Now Downloading new Firmware for target device: PIC32MX470F512H
        Downloading AP...
        AP download complete
        Programming download...

        Currently loaded firmware on ICD 3
        Firmware Suite Version.....01.38.10
        Firmware type..............PIC32MX

        Programmer to target power is enabled - VDD = 3.300000 volts.
        Target device PIC32MX470F512H found.
        Device ID Revision = A0

1. On the line labeled "Source:", click Browse and locate the HEX file created earlier.
1. Click Program:

        2015-08-28T15:24:29+1200- Programming...

        The following memory area(s) will be programmed:
        program memory: start address = 0x0, end address = 0x247ff
        boot config memory
        configuration memory

        Device Erased...

        Programming...
        Programming/Verify complete
        2015-08-28T15:24:34+1200- Programming complete
        Pass Count: 1

## Programming a Mikro-e Clicker Board using AVRDude

Your Mikro-e clicker board can be programmed directly from your linux build environment VM. This process uses a special
bootloader that talks to the AVRDude image loader.

These instructions detail the process of loading the bootloader onto your clicker board and programming it with the
ButtonDemo application.

**Note:** In order to program your clicker board with the bootloader, you will need to source a compatible hardware
programmer tool such as Microchip's ICD3, or PicKit 3.

Let's get started....

**Pre-requisites:**

- Mikro-e PIC32MX clicker board.
- micro-USB cable (for powering the clicker board).
- PIC32MX-compatible hardware programmer tool (e.g. MPLAB ICD3, PICkit 3, chipKIT PGM).
- Microsoft Windows workstation with the Git command line tools installed.

### Building and Programming the Bootloader

We'll first start out by checking-out the bootloader code and building it for the mikro-e clicker board. This process
uses Microchip's free XC32 compiler for PIC32.

Using a Windows workstation, install the latest version of Microchip's MPLAB X IDE toolchain by downloading the latest
version from the Microchip website and following the on-screen instructions. The toolchain is available from the following
link under the '*Downloads*' section:

    http://www.microchip.com/pagehandler/en-us/family/mplabx/home.html

Assuming you have a suitable (see above) hardware programmer tool on hand, plug your hardware programmer tool into the
'*MikroProg*' (PICkit) header on your clicker board .

**Make sure to observe the correct polarity of the programming cable with respect to the programming header.**

Incorrectly connecting this could cause damage to the clicker board or programmer tool. Pin-1 can be identified by the
small arrow on the underside of the clicker board.

Plug the clicker board into a free USB port of your workstation to power it. Turn on the clicker board by sliding the
power switch to the 'ON' position. If necessary, wait for the Windows drivers to be installed for the programmer tool.

Next, in Windows, check-out the *PIC32-avrdude-bootloader* project from GitHub

    git clone https://github.com/seank-img/PIC32-avrdude-bootloader.git

Start MPLAB X IDE, and open the *PIC32-avrdude-bootloader* project.

Click "*File*" -> "*Open Project*" and navigate to the checked-out project directory.

The project file itself can be found in *./bootloaders/chipKIT-Bootloaders.X*

After the project has finished loading, make sure it is set as the active project by right-clicking 'chipKIT-Bootloaders'
in MPLAB X IDE's 'Projects' pane and clicking 'Set as Main Project'.

Select the '*MIKROE_6LOWPAN_CLICKER*' build configuration from the dropbox (next to the hammer icon) at the top toolbar
of MPLAB X IDE. This needs to be correctly set in order to build for our intended target the clicker board.

Next, right-click the 'chipKIT-Bootloaders' project in the Project pane again and click 'Properties'. Highlight the
"Conf: [MIKROE_6LOWPAN_CLICKER]" entry in the Categories list and select the correct hardware programmer tool for your
programming setup from the 'Hardware Tool' box. Click OK.

Finally, kick-off the building and programming process by clicking the "Make and Program Device" button (chip icon with
a downwards facing arrow).

Wait until you see the text "*Programming/Verify complete*", indicating the build and programming process was successful.

You should also notice the leftmost red LED on the clicker (LD1) is flashing, indicating the bootloader is running correctly
and waiting for its first firmware image to be downloaded.

**Note:** At any point in the future, you can always put the clicker board back into its bootloader mode by power-cycling
the bpard and holding down the left button at the same time.

### Loading a Firmware Image Onto the Clicker using the AVRdude Bootloader

**Pre-requisites:**

- Clicker board with PIUCD32-AVRDude-bootloader pre-programmed.
- Micro-USB cable (for powering clicker).
- Ubuntu (or similar) Linux development VM.

Next, we'll load a firmware image onto the clicker board.

To do this, we need to install the USB extensions for Virtual box to allow the VM to access the USB device. Download
'Oracle_VM_VirtualBox_Extension_Pack-4.3.34-104062' from http://download.virtualbox.org/virtualbox/4.3.34/.
Install the USB extensions and restart the VM.

When building contiki from scratch, delete the file 006-remove-bootloader.patch. Deleting this file enables the bootloader.

Install Avrdude:

    $ sudo apt-get install avrdude

Add user to 'dialout' group:

    $ sudo adduser $(whoami) dialout

Log out and log in again (for the group change to take effect).

Turn on the clicker board device, holding down button T1 to enter avrdude bootloader mode. LED LD1 will flash repeatedly.
Right-click the USB-plug icon in the bottom-right of the Virtual Box window and select 'www.cpustick.com Stk500v2' to
forward the USB device through to the VM.

Next we need to add pic32 definitions to AVRDude. these are found in ".avrdudrc". Copy this file to your home directory.

    $  cp contiki/.avrduderc ~/

Run Avrdude with a contiki hex file, for example the button app:

    $ avrdude -p pic32 -c stk500v2 -P /dev/ttyACM0 -U flash:w:lwm2m-client-contiki-button.hex

While programming, LED LD1 will flicker and LED LD2 is solid on. When finished, both LEDs turn off and the device will
automatically boot.

## Using the LWM2M Contiki Client

### Connecting the LWM2M Contiki client to a CI40

WIP

### Connecting the "Simulated" LWM2M Contiki client to a Gateway server

Assuming you have built both the gateway server/bootstrap and the constrained
device client as outlined in the sections above.

Start the simulated Constrained device client:

    $ cd lwm2m-client-contiki
    $ sudo ./lwm2m-client-contiki.minimal-net

From a new terminal session:

    $ sudo ifconfig tap0 add 2001:1418:100::1

Start the gateway server:

    $ build/core/src/server/awa_serverd --verbose --interface tap0 --addressFamily 6

From yet another terminal session, start the bootstrap server:

    $ build/core/src/bootstrap/awa_bootstrapd --verbose --interface tap0 --addressFamily 6 --config core/bootstrap-contiki.config --port 15683

If successful, the client will stop printing "No response to client initiated bootstrap" and the ''awa-server-list-clients''
tool will show a connected client called 'imagination1'.

### Connecting the "Simulated" LWM2M Contiki client to a Gateway server and FlowCloud

If tayga is not installed, install it with:

    $ sudo apt-get install tayga

Start the simulated Constrained device client:

    $ cd lwm2m-client-contiki-flow
    $ sudo ./lwm2m-client-contiki-flow.minimal-net

From a new terminal session run the tayga setup script. Note "enp0s9" is the interface with an IPv4 connnectivity to the
Cloud LWM2M server and Tayga requires that the directory "/var/db" exists.

    $ cd scripts
    $ sudo ./tayga.sh -a 2001:1418:100::1 -a aaaa::1 -e enp0s9

Start the server in another terminal session.

    $ build/core/src/server/awa_serverd --verbose --interface tap0 --addressFamily 6

Start the bootstrap server in yet another terminal, note the two bootstrap config files. "bootstrap-contiki.config" specifies
the gateway server and "bootstrap-contiki-flow-NAT64.config" specifies the NAT64 translated address of UAT.

    $ build/core/src/bootstrap/awa_bootstrapd --verbose --interface tap0 --addressFamily 6 --config core/bootstrap-contiki.config --config core/bootstrap-contiki-flow-NAT64.config --port 15683

Manual process to provision with built-in tools

    $ ./build/tools/awa-server-define -o 20000 -j FlowObject -y single \
      -r 0 -n DeviceID -t opaque -u single -q mandatory -k rw \
      -r 1 -n ParentID -t opaque -u single -q optional -k rw \
      -r 2 -n DeviceType -t string -u single -q mandatory -k rw \
      -r 3 -n Name -t string -u single -q optional -k rw \
      -r 4 -n Description -t string -u single -q optional -k rw \
      -r 5 -n FCAP -t string -u single -q mandatory -k rw \
      -r 6 -n LicenseeID -t integer -u single -q mandatory -k rw \
      -r 7 -n LicenseeChallenge -t opaque -u single -q optional -k rw \
      -r 8 -n HashIterations -t integer -u single -q optional -k rw \
      -r 9 -n LicenseeHash -t opaque -u single -q optional -k rw \
      -r 10 -n Status -t integer -u single -q optional -k rw
    $ ./build/tools/awa-server-write -c imagination1 -o /20000/0 /20000/0/2=FlowConstrainedDevice /20000/0/6=17 /20000/0/5=<FCAP>

The server provisioning tool needs to be built - TODO

Now the provisioning tool can be used to provision the Simulated Contiki client:

    $ awa-server-provision --fcap <FCAP> --clientID imagination1
