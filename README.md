Beauty and the Beast Lightbox
===

This is the Arduino code for my custom Beauty and the Beast lightbox. It started from [this thing](https://www.thingiverse.com/thing:4816684). I printed the box (at 75% width and height) but had trouble printing the scene blades. Eventually, I gave up and used my partner's Cricut with some cardstock for the blades. The blades are made from 85 lb white cardstock with 5mm craft foam sheets cut into strips and glued around the edges to separate them. The front is a glass pane cut to size and secured to the front with silicone (this was a huge pain in the ass, I would look for a better way to do it in the future). Behind the glass sits the stack of scene blades and behind that is a ring of WS2812 individually addressable RGB LEDs in a 30 light per meter strip. It ends up being 16 lights total around the perimeter. The strip has built-in adhesive but that's not quite sticky enough so I ended up hot-gluing it in. I've attached all of the electronics to the back plate, and arranged it rather poorly and hot-glued it all down, but it works.

As for electronics, I've used a 5v, 3A power supply (the cord is way too short, need to find a better one for future projects) to supply power to the whole thing, an Arduino Nano Every handles all of the logic, and 3 momentary push buttons provide controls. I purchased the power supply before reading the Arduino specs close-enough so I had to add in a cheapo voltage booster to provide Vin from the 5v supply. I probably could have removed the on-board voltage regulator to eliminate the booster but I didn't want to mess with that for this project. I'll think about it for next time. Additionally, the music is played using a typical piezoelectric buzzer I had lying around from my college days. Still works great and sounds terrible. The buttons are all wired together on one side to a single pin on the Arduino with the other side of each button wired to its own pin on the Arduino.

# Software
Software-wise, there are three main components:
1. The main loop which coordinates all updates and looks for button presses
2. The music generation
3. Light controls

## Main Loop

On startup, we do a few minor setup things:
1. Seed the random number generator
2. Tell the Adafruit NeoPixel library about our LED strip
3. Set the pins for button reading
4. Prepare the timer peripheral for our wave generation

Then we let the main loop run wild. It does the following:
1. Check for button presses.
2. Tell the lighting system to update
3. Check if we should update music. (A lot of this could be moved to the music system the way I did for lighting, but it works for now so I'm not going to mess with it for this iteration)
    1. If enough time has passed for a music interval to run and music is currently playing, tell the music system to update.
    2. If the music system says the song is complete, turn off music and reset the light system
4. If music is not playing:
    1. If lights are not on, check if the sleep timeout has passed. If so:
        1. Set the buttons to interrupt mode
        2. Go to sleep
    2. If lights are on, check if it's been an hour. If so, stop lights
    3. Delay 10ms

## Buttons

The buttons are wired with a single common pin on one side for all three buttons and an individual pin for each on the other side. I read them in two different ways depending on the state of the chip. I mostly followed [this guide](https://create.arduino.cc/projecthub/Svizel_pritula/10-buttons-using-1-interrupt-2bd1f8) for setup and used some of the code with some modifications for my specific needs. When the chip is running normally, I always use the distinct setup (common pin outputs low and individual pins are pulled up and treated as input) and do a manual check on each loop. This is because the lighting system disables interrupts and so it would cause the buttons to respond inconsistently. When we go to sleep, the configuration is changed to the common-pin-mode which allows us to listen for the interrupt to wake the chip and quickly switch to distinct mode to read the input. Since starting the project, I've realized that this was probably not necessary since the 4809 chip in the Nano Every supports interrupts on every pin. I may change this in a future revision.

The three buttons are as follows:
1. Turn on a light show in which lights cycle through 5 patterns, one per minute for an hour.
2. Play the BatB dance song with a special light pattern.
3. Turn off sounds and lights.

I wired them this way larely for lazy reasons. I didn't feel like figuring out debouncing and this setup with idempotent inputs doesn't require it.

## Lights

The lighting system uses the Adafruit NeoPixel library to control the WS2812 LED strip. The LEDs are powered directly from the power supply because they can pull quite a lot of power and I didn't want to risk them over extending the voltage regulator on the Arduino. The Arduino uses a single pin to provide commands to the LED strip.

The NeoPixel library requires extremely precise timing to control the lights properly so it has to disable interrupts when sending commands. Unfortunately, the built-in `tone` function provided by the Arduino API uses timer interrupts to create the tone waveform. The disabling of the interrupts caused the music to be garbled whenever lights were on at the same time, a key requirement for this project. As such, I had to do some research and found through [this article](https://learn.adafruit.com/neopixels-and-servos) that an on-chip timer peripheral could be used to create a square wave that would require interrupts. Unfortunately, no one seems to have written a library similar to the servo library in that example specifically for tone generation, so I had to do some experimentation. I may try to make this more general and release it as a separate library but it would likely take some effort.

Lights are currently configured to update at 60 frames per second which produces very smooth, pleasant animations. Each loop, the lighting system is triggered and it internally decides what should be shown.

The coordination of what to show is largely based on two variables, `currentProgram` and `currentPattern`. `currentProgram` controls whether we are currently in a light show without music, during which the pattern will change every so often, the special music pattern, during which we show the same pattern continuously throughout the song, or off. `currentProgram` dictates what individual light pattern will be shown on each loop. Additionally, each variable has a "transition" state which allows the lights to blend from one pattern to another. 

When the `startLights` call is made, a Program is passed in. This will be the program we enter the lights into. Turning off the lights is considered a program. This allows us to do a smooth transition from lights to no lights without the outside having to know what's happening. Instead of immediately setting the program, the current program is set to `ProgramTransition`, with the `nextProgram` and `nextPattern` set according to the requested program. The transition pattern is then run which does a smooth LERP over hue, saturation, and value from the current state to the intial values of the next pattern over the course of a second.

The LED settings are controlled via two arrays of structs. The struct is simply a collection of hue, saturation, and value settings, each a double from 0.0 to 1.0 (double is probably overkill, but this is simple enough that it doesn't matter). One array stores the current LED value during normal operation. The second is to store the first state of the LEDs for the next pattern to allow for a smooth transition. These arrays are global to the module.

Each update to the light system goes through the following steps:
1. Check if the current Program is `LightsOff`. If so, return, no lights.
2. Check if the current pattern should change
    1. Is the current program `WithSong`? If so, do not change program
    2. If the current program is `Standalone`, check if the time since pattern change is greater than the pattern time. If so:
        1. Move to the next pattern.
        2. If we've gone through all normal patterns, return to the start of the list.
        3. Initialize the new pattern
        4. Reset timers
3. If the current program or pattern is `Transition`, run the transition update
4. If not, run the normal update
5. Send light changes to the LED library
6. Reset interval timer

### Pattern Logic
Each pattern (except for the special `LightsOff` pseudo-pattern) consists of two functions, initialize and update. The initialization function is run when we first switch to the pattern, either on starting the light system, or after a pattern change during the `Standalone` program. This function should set the very first state the LEDs will be in into the shared `transitionLightsHSV` array. Once this is done, the transition update system will automatically create a smooth LERP between the last state of the previous pattern and the first state of the next pattern. After the transition is complete, the transition update will move the transition values to the current values array and control will be passed to the next pattern.

The update function is not passed any parameters and should take all needed settings from global timers or its own specific constants. The update function is expected to set the new light settings into the `currentLightsHSV` array. The update system will automatically take those settings and convert them to the values needed for updating the LEDs.

Turning off the light system uses a special program called `LightsOff` which has a pseudo-pattern in which all lights are off. This pattern only has an initialization function because the only updates necessary are handled by the transition system automatically.

This entire system could and should be refactored to a more extensible setup.

## Music

The music system is built around an array of notes, each storing a frequency to play and a number of intervals to play it. In my simple implementation, the shortest note that can be played is an 8th note. I used a few different sources to get the correct notes. First, I found a midi arrangement of the song [here](https://beautybeast.enchanted-rose.org/multimedia_midifiles.php). Second, I isolated the vocal track into a separate midi file using MuseScore. I then fed that midi file into [this web app](https://extramaster.net/tools/midiToArduino/) which produced an Arduino sketch that plays all of the notes. Unfortunately, it has some issues and the delay times it uses are extremely long. I eventually figured out how to map the times it produces to typical note lengths and converted the frequencies into some note constants for readability. This allowed me to produce an array of frequency/length pairs which I could read and play.

The system is configured to track 4 "intervals" for every eighth note. It keeps track of how many intervals have passed since we changed notes and triggers the wave generator to change frequencies once the current note has been playing for long enough. In the cases when the frequency doesn't change between notes, I insert a very short (4ms) delay with no sound to create an audible break. It sounds much better this way. The code originally did this on every note change but it was causing audio issues and was imperceptible on normal changes.

There is currently still an occasional hiccup in the sound that I have not been able to figure out. It happens on seemingly random notes and only at most once or twice per playthough (usually not at all). It's similar to the issues that occured when the short inter-note delay was happening every note change, but I have not heard it happen during the few instances where this delay is still used. My hunch is that it's happening when the change of a note coincides with a light update on the same loop but have not tracked it down yet. Debugging this is difficult because writing to the serial port is very slow.

Currently the sound interval timing is tracked via the main loop, but it should be moved into the sound system.

## Cautions
As noted in a couple of places, some of this code is very specific to the Arduino Nano Every (or Due Wifi Rev 2 which uses the same chip). The current pin settings also require a specific update to one of the Arduino API source files that I will be logging with the MegaAVR Arduino library and opening a PR.