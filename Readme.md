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

Finally, you set `statusTopics` to a list like the one above. This will cause
the service to report on the current high/low state of eight further pins on the
raspberry pi (that state detection mentioned in the introduction). The logic is
the same as for `toggleTopics`, in that the order defines which input pin will be
set. This will be checked and published on startup, to ensure that the MQTT broker
has up to date information, even if it has been asked to retain the most recently
published state. By convention, the endpoint should be the same name as the toggle
topic, except that it should end with status (so some/mqtt/topic/four/toggle and
some/mqtt/topic/four/status would correspond to toggling and containing the state
for the same remote relay).

The value published to the status topics are "on" and "off", for whether the
remote relay is closed or open respectively (that is, equivalent to pushing a
button to close the circuit will cause "on" to be published, and not having the
button pushed will cause "off" to be published).

#### Alternative Topic Definition

Instead of the two lines of topics above, you can also construct them in a more
descriptive manner, by adding a section structured like the one below to your
configuration file:

```
[Topics]
topicBase=some/mqtt/topic
toggleEndpoint=toggle
statusEndpoint=status
topic-1=kitchen
topic-2=livingroom
topic-3=bathroom
topic-4=attic
topic-5=bedroom
```

The above will be equivalent to having the two lines for toggle and status
topics in your configuration, with five topics in each, in the form of e.g.
some/mqtt/topic/kitchen/toggle for the first topic's toggle entry.

Using this method expects you to have the status endpoint set up. If your toggle
and status endpoints are toggle and status respectively, you can leave them out
of the configuration file, as those values are the default. Note also that your
topics must be listed in numerical order, that they are 1-indexed, and that if
there is a hole in the list, the remainder will not be read (so say you have a
list of topic-1, topic-2, topic-4, and topic-5, then the parser will ignore
topics 4 and 5, as there was no 3 listed).

You can leave out topicBase if you have different paths to the various parts
if you need them, and you can add sub-paths in the topics as well (if, for
example, you have a zoned setup, you could have topics named upstairs/bedroom-1
and downstairs/bedroom-1 if you wanted).

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

## Our Own Setup

Wanting to get the technical stuff out of the way first, here, then, is the
description of what we are in fact controlling using this. We decided that what
we wanted out of lighting control was a system which was able to be made smart,
but which would also fail all the way back to "push button get light". What we
came up with, then, was to use 24V DC controlled latching relays with two
switched channels. One of the channels could supply the lights with 240v, and
the other could be used independently to test the state of the relay from some
external device.

The 24V DC switching power was less important here, but it would allow us to
run long enough cables to reach the light switches from our central box full
of many relays, without having to call in an electrician to do so, by leaving
the mains side untouched. It also makes it safer to switch from the Waveshare
8-channel Relay Board, which, while it does handle 240v, it's of course nicer
to not have to deal with mains if we can get away with that.

Specifically, the relays we use are Finder's model 20.21.9.024.4000, and the
switching power is provided by a DIN-mountable power supply by Phoenix called
STEPPS1AC24 #2868622.

What relayboard-control gives us, then, is both the momentary pulse that would
cause the relay to switch its state, and the detection side which uses the
unused second side of the relay to detect whether the lights are on or not, and
then it exposes both of those through an mqtt broker, which can be interfaced
with through Home Assistant (or, i guess, any other MQTT based thing). In
short, it allows for our dumb light switches to become super-extra-smart.

Now, the final trick here is that, we fitted those relays and the push-button
wall switches at the end of 2019, and as i am writing this in December 2021, i
am glad to confirm that the intention that it should fall back to "push button,
get light" has worked a treat, as that is precisely what happens. We have yet
to try the critical fallback of "oh no the power supply failed" where we would
have to leave the control box open and push the button on the relays
themselves, but since that is a basic function of the relays themselves, we
have no doubt about whether or not that would work (or, well, function at
least, given that would be at most a temporary workaround).

### Home Assistant Bits

The final step you would need to use this tool, apart from setting up that mqtt
broker and hooking it up to your Home Assistant instance ( [see here for how to
do that](https://www.home-assistant.io/integrations/mqtt/) ), is to set up the
switches for Home Assistant to use.

As an example, here are the Home Assistant configuration bits for two of the
eight switches we have configured at our place (just so you can see how the
lists and whatnot should be done, as that confused me, a first-timer, with what
needed to be unique entries and what should be entries in a list or object
children or whatnot).

With these entries in your configuration file, and the mqtt integration set up,
two new switches will appear called Apex Up Light and Apex Down Light, and you
can stick them anywhere you would like in the Lovelace UI.

#### configuration.yaml:

```
binary_sensor:
  - platform: mqtt
    name: Apex Up Light
    unique_id: binary_sensor.apex_up_light
    state_topic: "foxhole/lights/apex-up/status"
    payload_on: "on"
    payload_off: "off"
  - platform: mqtt
    name: Apex Down Light
    unique_id: binary_sensor.apex_down_light
    state_topic: "foxhole/lights/apex-down/status"
    payload_on: "on"
    payload_off: "off"

switch:
  - platform: template
    switches:
      light_apex_up:
        friendly_name: Apex Up Light
        unique_id: switch.light_apex_up
        value_template: "{{ is_state('binary_sensor.apex_up_light', 'on') }}"
        turn_on:
          service: script.toggle_apex_up_light
        turn_off:
          service: script.toggle_apex_up_light
      light_apex_down:
        friendly_name: Apex Down Light
        unique_id: switch.light_apex_down
        value_template: "{{ is_state('binary_sensor.apex_down_light', 'on') }}"
        turn_on:
          service: script.toggle_apex_down_light
        turn_off:
          service: script.toggle_apex_down_light
```

#### scripts.yaml:

```
toggle_apex_up_light:
  alias: Toggle Apex Up Light
  sequence:
  - service: mqtt.publish
    data:
      topic: foxhole/lights/apex-up/toggle
  mode: single
  icon: mdi:wall-sconce-flat-variant
toggle_apex_down_light:
  alias: Toggle Apex Down Light
  sequence:
  - service: mqtt.publish
    data:
      topic: foxhole/lights/apex-down/toggle
  mode: single
  icon: mdi:wall-sconce-flat
```
