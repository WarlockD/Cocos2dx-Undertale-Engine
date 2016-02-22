# Cocos2dx-Undertale-Engine
Cocos2dx Engine  with some simple Lua bindings, that makes it easy to mess with the Undertale to create mods or new battles

So far I have it loading the UNDERTALE.EXE, extracting the data.win file then loading the textures and 
sprites into the hellow world screen.  Should be able to compile it by just loading up the win32 2015 solution.

OK!  We sort of have an interface now.  You can move Chara around using the arrow keys or the WASD keys.  
Press the '+' and '-' by the keypad to cycle though all the rooms, press '*' on the keypad to turn and off all non-visible objects.
Press 'Print Screen' or F12 to screenshot
And press 0-8 to turn on and off individual backgrounds.

At this point I need to work on the collision side more and put in a lua code system for each room.  I have been half toying with a bytecode emulator for game maker ( from my perspective, it be much easyer than decoding EACH room/object ) but I am unsure the legal grounds at this point.
