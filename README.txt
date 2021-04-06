This code builds on excellent work by jhaines0 to receive and decode signals from Honeywell 5800-series wireless sensors using an inexpensive software defined receiver.

It is being developed and tested on a Raspberry Pi3 running Rasbian with a NooElec NESDR SMArt RTL-SDR, but should work on any Linux-based system with a compatible RTL-SDR and appropriate libraries.

The intent is to receive signals and forward events to another application.

******************************************************************************************************************
WARNING: This software and information accompanying it is intended for educational and experimental purposes only.
Under no circumstances should it be relied upon for protection of persons or property.
******************************************************************************************************************

Background

The Honeywell 5800 series transmit a serial number and the status of up to 4 loops when a state change occurs, and every so often for supervision. There is no indication of what kind of sensor it is. The panel is programmed with a sensor type and response type for one or more loops, but it has no way to know what the device is. That makes the system more flexible, but also more difficult for a third-party decoder to interpret.

By design, the alarm panel ignores all unknown serial numbers as well as loops that are not programmed. There is one exception: the panel assumes loop 4 as tamper for some device types.

The status byte bitmap is as follows:

Loop 1 - 0x80
Loop 2 - 0x20
Loop 3 - 0x10
Loop 4 - 0x40

0x04 is set for supervisory messages (i.e. check-ins without a sensor state change)
0x02 is low battery indication.

Most 5800-series wireless motion detectors report on loop 1, but some use loop 2 for a separate feature (such as a pet immune motion option). Some have tamper on other loops, some don't. The basic 5800PIR only uses loop 1.

Smoke detectors, such as the 5808, report on loop 1. The 5808W3 uses loop 2 for a maintenance warning, loop 3 for low temp, and loop 4 for tamper.

The 5853 glassbreak reports on loop 1 and tamper is loop 4.

The common 5816 window/door sensor reports the wired NC loop (there are screw contacts inside the case) as loop 1, the magnetic contact as loop 2, and the built-in tamper switch as loop 4. Most installers just ignore the wired loop and don't program it on the panel, so you'll see it report loop 1 and 2 when the door/window opens, and just loop 1 when it closes. However, it is common to use loop 1 for retrofit/takeovers or when the 5816 dosn't fit a door or window.

The 5816 can be used for many other purposes. For example, if you connect an NC panic button to the wired contacts, loop 1 becomes a panic indication. Or, you can program 5816's serial number into the system as a keyfob, assign loop 1 as "Arm Away", and you now have a button to press before leaving that can be used to arm, but not disarm, the alarm system.

The 5818MNL concealed magnetic contact switch reports as loop 1. When used as a perimeter device, it is usually programmed as a normal door contact. However, in homes where pets are present and PIRs create too many false alarms, it is not uncommon to install this on an interior door (such as a home office or master bedroom) and program the panel as if the device was a motion sensor. (This causes it to be ignored when the alarm is armed in "stay" mode, etc.)

The 5822T Tilt Switch has wired contacts (NC) on loop 1 and a tilt switch on loop 3. It is mostly used for overhead garage door monitoring.

The 5281 Temp and Flood sensor usually uses loop 1 for cold (ie. pipes will freeze if you don't fix the furnace soon) and loop 3 for flood (water). But it has DIP switches that allow other configurations, such as too hot or freezer failure conditions (with an external probe) on loop 2. Loop 4 is tamper.

The 5817XT supports a programmable NO or NC wired on loop 1, reed switch on loop 2, and wired NC on loop 3. For life safety applications, or those who like to tinker, loop 1 can be configured to transmit an alert every 4 seconds until the loop is restored. Loop 4 is tamper.

The 5834-4 four-button remote reports each button as a different loop, and combinations of buttons as a second set of four loops with the serial number incremented by 1. Note that the "*" button (lower right) is loop 1, and they rotate counterclockwise.
