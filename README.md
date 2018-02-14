# UWOT
My implementation of Ben Tristem's tank game following his [UE4 course](https://www.udemy.com/unrealcourse/).
If you want to reimplement features similar to this demo, the fastest way would just be reading through the source code and the tank setup in my implementation. However, in case you do not like to shoot in the dark, or would rather reimplementing from the raw, I try to provide as many references that I have used during the development below. Enjoy :3

# [Demo Game Download:](https://goo.gl/Ps3ZJw)

# Feature listing:

## PhysX tank vehicle:

### Demo:
[![Alt text](https://img.youtube.com/vi/V73rDNuEIJQ/0.jpg)](https://youtu.be/V73rDNuEIJQ)

### References:
- [PhysX's vehicle manual](http://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/Vehicles.html): You should read this one to take a good grip as to how physx vehicle does things. You can reuse UE4's vehicle setup for most of the c++ implementation though. The important thing is that the article does provide a lot of hint on tuning the vehicle though. Tuning the tank is a real pain, so follow these hint will save you lots of time.
- [BoredEngineer's Tanks, tracks and N-wheeled vehicles](https://forums.unrealengine.com/community/work-in-progress/54936-assets-open-source-tanks-tracks-and-n-wheeled-vehicles?83483-ASSETS-OPEN-SOURCE-Tanks-tracks-and-N-wheeled-vehicles=): This one is 100% blueprint tank implementation, and a great one at that. This has example on creating animated track using spline and instanced static mesh, on which mine is pretty much just a C++ reimplementation.
- [Fixing the Vehicle class in UE4](https://forums.unrealengine.com/development-discussion/c-gameplay-programming/65584-fixing-the-vehicle-class-in-ue4-starting-with-nwheel-implementation) A good thread that has a lot of details that you can use to tune your tank vehicle.

## Camouflage:
Camouflage with transition effect. I use Dither Temporal AA to simulate camouflage effect. I tried both translucent and screen texture emissive material but both gave weird artifact when using with multi-layer mesh. Dither Temporal AA also result in some noise on the surface, which looks rather reasonable for camouflage.

### Demo:

- **Material demo**

![Material demo](https://github.com/megafirzen/UWOT/blob/master/RawContent/Images/Demo_Camo001.gif "Material demo")

- **In-game demo**

![In-game demo](https://github.com/megafirzen/UWOT/blob/master/RawContent/Images/Demo_Camo002.gif "In-game demo")

### References:
- [TGA Digital - UE4 Transition Effect Shader](https://youtu.be/_vGLVXHEQDQ)
- [Tech Art Aid - UE4: How to fix translucent materials (dithered opacity)](https://youtu.be/ieHpTG_P8Q0)
- [Tech Art Aid - UE4: Stealth Invisibility Effect (With Distortion)](https://youtu.be/9ZawosRVZrs)
- [UnrealCG - Disintegration Effect- UE4 Material Function Tutorial](https://youtu.be/gldIJGqlWf0)

# References:
## Tutorials:
- [Tom Looman's Outline effect](http://www.tomlooman.com/outline-effect-in-unreal-engine-4)
- [PawelM's Outline effect, with explaination on Sobel operator](http://www.michalorzelek.com/blog/tutorial-creating-outline-effect-around-objects)

## Materials:
- Sixthsense/Newtype flash SFX and Char Aznable image are owned by Sunrise Inc. and Bandai Inc.
- Epic's Vehicle Game demo: You can find it under "Learn" subtab of Epic Game Launcher - Unreal Engine Tab.
- [Main BGM: Matthew Pablo - Dream Raid I](https://opengameart.org/content/dream-raid-cinematic-action-soundtrack)
- [Reload SFXs - Drawer SFXs - Mafon2](https://freesound.org/people/Mafon2/sounds/330021)

# [Backlog](https://trello.com/b/TEaTh0oi/uwot)
