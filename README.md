# Dotty
A simple, arcade-like highscore-based game that takes a **lot** of inspiration, code-wise, from [Terri-Fried](https://github.com/PolyMarsDev/Terri-Fried/).
![Dotty gameplay](https://raw.githubusercontent.com/AtlasC0R3/dotty/main/gameplay.gif)

You control a little square named Dotty, and you have to guide him towards his dots. 
There are even potions along the way to help you: a potion that duplicates dots, and another that clones Dotty.
Sounds simple, right? Well, let's complicate it a bit.
You have to avoid hitting the edge of the screen (or window), the death potions, and hitting your clone.

*Fun fact: I had the idea for this game for almost 2 years (2019, maybe 2018, I don't remember), but have never made it into an actual game*

## How do I compile this?
This game uses [raylib](https://www.raylib.com/) as a graphics/sound/input library, so I suggest you build raylib 
([Windows](https://github.com/raysan5/raylib/wiki/Working-on-Windows), [macOS](https://github.com/raysan5/raylib/wiki/Working-on-macOS), [Linux](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux), [etc.](https://github.com/raysan5/raylib/wiki))
then use make (or mingw32-make, or tcc, asjhfggjzad) to compile.

*Tested on Windows 10, Debian and Android 10 (LineageOS 17.1 on a Samsung Galaxy S6 Edge). [Note that Android builds may or may not work for you](https://github.com/AtlasC0R3/dotty#this-doesnt-work-on-android-dotty-keeps-closing).*

## F.A.Q.
well should i explain what a faq is

### I want to port this project, but can't find the font that this uses
This uses raylib's default font, which is its own font. (that definitely is a sentence.) There is a very similar font called "Grixel Acme 9" ([link](https://www.dafont.com/grixel-acme-9.font), [official grixel website](https://www.grixel.gr/)) which you can use.

### This doesn't work on Android! (Dotty keeps closing)
So far the raylib-compiled apks only run on Android 10 and up. I can't exactly figure out what's causing this issue. *Sorry for the inconvenience.*

### I'd like to implement a new item in this game/I have an idea for this game
short answer, no.

Long answer: Dotty was meant to be a simple, high-score game, that anyone could pick up and understand how to play 
(left, right, up, down. get all dots, get potion upgrades, avoid edge of screen, avoid death potion, avoid hitting clone). If I were to add new ideas, then it wouldn't be as simple.
I definitely could add new items, but that would require a new version/branch of Dotty.



Copyright (C) 2021 atlas_core
