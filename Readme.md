# relayboard-control

A basic, MQTT integration point service for the Waveshare 8 channel relay board.

This was built specifically for our own home's relay based lighting control solution,
where this relay board is used to pulse external 240v two-channel relays, whose first
channel is used for the supply for the lights, and the other for state detection read
by the raspberry pi connected to the relay board, but it feels like the sort of thing
which is just too useful to keep sat just in our home.

To use this, you will need a raspberry pi (we use a RPi2b, because it doesn't do
anything else), and a Waveshare 8 channel relay board, as can be seen here:

https://www.waveshare.com/product/modules/others/power-relays/rpi-relay-board-b.htm

## Building

To actually build this tool, you will need:

### Some packages

This all assumes that you are running Raspbian Buster, so that would be the first step.

Next step is to install some packages that are needed to build everything else:

```
sudo apt install qtbase5-dev extra-cmake-modules libkf5config-dev
```

###  QtMqtt, the Qt MQTT library

This is not available on the Raspbery Pi repositories, so you will need to build it
yourself. Easy enough to do, but there's a couple of fun wiggly bits that need
taking care of (such as ensuring you use the correct branch, and in some cases you
will need to adapt the .qmake.conf file if it thinks you are using the wrong
version of Qt - i did not need to do this on the pi, but had to do it on my
desktop, so mileage may vary).

- clone git://code.qt.io/qt/qtmqtt.git
- change to branch 5.11
- sudo apt install qtbase5-private-dev
- qmake -r && make && sudo make install

### bcm2835, the GPIO c library

Instructions can be found here - simple and straightforward, except that you do have
to build it yourself for whatever reason.

http://www.airspayce.com/mikem/bcm2835/

### Building relayboard-control

Clone the repository somewhere useful, like the pi home directory:

```
cd
git clone (this repository)
cd relayboard-control
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
```

Finally to install the tool and the systemd unit, just do the usual dance:

```
sudo make install
```

## Running

Before running, you will need to set up the MQTT integration bits. For this, you
will need to have an MQTT broker set up already, and know the address of it.

### Configuration

Once you have MQTT set up, you need to create a configuration file called
`/etc/relayboard-control.rc` and in that file you will need at least three lines:

```
[General]
toggleTopics=some/mqtt/topic/one/toggle,some/mqtt/topic/two/toggle,some/mqtt/topic/three/toggle,some/mqtt/topic/four/toggle,some/mqtt/topic/five/toggle,some/mqtt/topic/six/toggle,some/mqtt/topic/seven/toggle,some/mqtt/topic/eight/toggle
mqttHost=your.mqtt.host.address
```

`toggleTopics` are the topics which the MQTT client in relayboard-client will
subscribe to. Anything published to those topics will cause the relay with the
corresponding number in the order to be pulsed on for 50ms (so for example,
publishing `some/mqtt/topic/one/toggle` will cause relay 1 to be pulsed). Note
that the names are not important, only the order in which they are listed.

`mqttHost` is the network address of the MQTT broker you wish to connect to.
You can give an IP, or you can give a normal hostname here. If you need to also
specify a port, use the `mqttPort` setting to do so.

This will set up the core functionality of relayboard-control, but there are
further options that you can (and likely want) to set:

```
mqttPort=1833
mqttUsername=yourusername
mqttPassword=yourpassword
```

(yes, the password is in clear text - treat this as a single-user login or an api
key: don't share this password between other accounts, and only use this user for
this single purpose, that way you can easily discard only the one should it end up
compromised for some reason)

The port shown in the example above is the default set by MQTT, but as that is
unencrypted, the recommendation is to change this to something which is. It will,
however, work without.

Finally, you set `statusTopics` to a list line the one above. This will  cause
the service to report on the current high/low state of eight further pins on the
raspberry pi (that state detection mentioned in the introduction). The logic is
the same as for `toggleTopics`, in that the order defines which input pin will be
set. This will be checked and published on startup, to ensure that the MQTT broker
has up to date information, even if it has been asked to retain the most recently
published state. By convention, the endpoint should be the same name as the toggle
topic, except that it should end with status (so some/mqtt/topic/four/toggle and
some/mqtt/topic/four/status would correspond to toggling and containing the state
for the same remote relay).

### Enabling the systemd unit

The systemd service is installed by the install command above, but to actually
enable it for startup when the pi boots up, you will need to ask for that to
happen explicitly, like so:

```
sudo systemctl enable relayboard-control
```

If you just want to run it, you can do so also using the standard commands for
operating systemd services (start, stop, status, and so on). The service will
output to the system log, which you can see using journalctl.
