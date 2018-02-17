# UWOT
My implementation of Ben Tristem's tank game following his [UE4 course](https://www.udemy.com/unrealcourse/).

# [Demo Game & Download](https://goo.gl/Ps3ZJw)

[![Alt text](https://img.youtube.com/vi/V73rDNuEIJQ/0.jpg)](https://youtu.be/V73rDNuEIJQ)

**https://youtu.be/V73rDNuEIJQ**

All future downloads will be gathered within [this folder](https://goo.gl/Ps3ZJw).

# For the adventures:

If you want to reimplement features similar to this demo, the fastest way would simply be reading through the source code and the blueprint tank setup in my implementation.

In case you do not like shooting in the dark, or would rather reimplementing from the raw, I try to provide as many leads/references that I have used during the development below. Enjoy :3

## Feature listing:

### PhysX tank vehicle:

#### References:
- You should take a look at UE4's **WheeledVehicleMovementComponent** and **WheeledVehicleMovementComponent4W** classes, especially the latter. I wrote my  **TankVehicleMovementComponent** by first made a copy of **WheeledVehicleMovementComponent4W** and then morphed it to support tank drive instead.

- [PhysX's vehicle manual](http://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/Vehicles.html): You should read this one to get a good grip as to how PhysX vehicle does things. UE4 already has their own PhysX wheeled vehicle implementation, and you can certainly build your tank implementation upon it like mine; or go full-hardcore and write your own from scratch using this article as starting point! [Nvidia GameWorks's PhysX's source](https://github.com/NVIDIAGameWorks/PhysX-3.4) will be a great lead if you choose the latter choice. You will need to register your github with the organization to view it however.
	
    This article also provides a lot of guideline on tuning vehicles. Tuning the tank is a real pain, so follow them will save you lots of time. I mean LOTS LOTS of time.

- [BoredEngineer's Tanks, tracks and N-wheeled vehicles](https://forums.unrealengine.com/community/work-in-progress/54936-assets-open-source-tanks-tracks-and-n-wheeled-vehicles?83483-ASSETS-OPEN-SOURCE-Tanks-tracks-and-N-wheeled-vehicles=): This one is 100% blueprint tank implementation, and a great one at that. This has example on creating animated track using spline and instanced static mesh, on which mine is pretty much just a C++ reimplementation.

- [Fixing the Vehicle class in UE4:](https://forums.unrealengine.com/development-discussion/c-gameplay-programming/65584-fixing-the-vehicle-class-in-ue4-starting-with-nwheel-implementation) A good thread that provides a lot of insights on how to fine-tune your vehicle.
 
- [Wikipedia - Tank Steering System](https://en.wikipedia.org/wiki/Tank_steering_systems)

- [Wikipedia - Slip](https://en.wikipedia.org/wiki/Slip_(vehicle_dynamics))

### Tips and tricks on tuning tank's steering:

Steering was probably the biggest headache to me when building the tank. As you may have known already, tank turns by differing their track speed on each side. PhysX allows you to control the amount of torque that is being transmitted from the engine to each side's by changing the thrust input (-1..1 range).

I am not sure by which mean that they reduce the ammount of torque. They can slow down the transmisison gear on one side and then the clutches will slow the wheels on that side to match with, or just simply reduce it straight from the equation of torque that the clutches transmit to the wheels (Which seems to be the case). Either way, controlling tank steering using this thrust input thingy is rather unwieldy. I couldn't accurately control the speed ratio between 2 sides at all. There are cases where reducing the thrust input does little effect on slowing the track down, so you would need to slow it down using brake. In short, it is too difficult to control the steering manually this way.

After struggling unsuccessfully with the thrust input. I decided to hack together a differential mechanism instead. First, we need to get the current speed ratio between 2 sides, then compare it with the desired ratio taken from control input. We can then increase/decrease the speeds of each side by setting the thrust input to 1 (increase) or -1 (decrease) or, say, 0 (keep current), respectively, to archive the desired ratio. You can find my implementation in:
```c++
void TankMovementComponent::SetSteeringDirection(FVector2D desiredSteeringDirection);
```

You will also need to fine-tune your blueprint tank properly. Some settings that I have found useful for steering tuning are:
- Vehicle's weight, Wheel's weight
- Engine torque curve: Jump the max torque to 5k-6k should do the trick if your tank weight about 20 tons like mine.
- Gear Ratios: The default gear ratios do the trick for me.
- [Clutch Strength](https://devtalk.nvidia.com/default/topic/764824/physx-and-physics-modeling/clutch-strength-question/)
- Wheel's Longitudinal slip stiffness: Increase this will help you turn faster while moving, but make it harder to turn in-place.
- Lateral slip stiffness: Keep this close to its default. Higher value will help limiting over-steering, but make it harder to turn.
- TireConfig's fiction scale: Increase it will certainly help your tank steer much more smoother, but will make turning in-place much harder (and will make your tank into some spider monster that can climb vertically)
- Moment of inertia settings: I generally do not want to touch these, as it is prone to leave some bad side effects (Such as sending your tank flying after collision, or down right crashing the game). If you have real-life data on these, them use them, but remember to always keep them above their defaults (>= 1).

### Camouflage:
Camouflage with transition effect. I use Dither Temporal AA to simulate camouflage effect. I tried both translucent and screen texture emissive material but both gave weird artifact when using with multi-layer mesh.

#### Demo:

- **Material demo**

![Material demo](https://github.com/megafirzen/UWOT/blob/master/RawContent/Images/Demo_Camo001.gif "Material demo")

- **In-game demo**

![In-game demo](https://github.com/megafirzen/UWOT/blob/master/RawContent/Images/Demo_Camo002.gif "In-game demo")

### References:
- [TGA Digital - UE4 Transition Effect Shader](https://youtu.be/_vGLVXHEQDQ)
- [Tech Art Aid - UE4: How to fix translucent materials (dithered opacity)](https://youtu.be/ieHpTG_P8Q0)
- [Tech Art Aid - UE4: Stealth Invisibility Effect (With Distortion)](https://youtu.be/9ZawosRVZrs)
- [UnrealCG - Disintegration Effect- UE4 Material Function Tutorial](https://youtu.be/gldIJGqlWf0)

## Spotting mechanism:

My spotting mechanism is based off of [WOT's one](http://wiki.wargaming.net/en/Battle_Mechanics#Spotting_Mechanics).

**Tl;dr:** Take a look at my **TankSpottingComponent** and **TankSpottingManager** classes.

Each **TankSpottingComponent** has a list of view range ports and checkpoints. The component will process ray-casting test against others with predefined viewrange/camouflage constraints once per random intervals, using its ports as start point and the others' checkpoints as target.

The testing-against-each-other part is rather cumbersome, so I actually create a singleton **TankSpottingManager** game object that keep references to all the **TankSpottingComponents** in the map and let it do all the testing heavy lifting instead. The **TankSpottingManager** will then notify all concerned **TankSpottingComponents** when a spotting event occur. The **TankSpottingComponents** now only need to contains their individual constraints and spotting event handlers.

### References:
- [WOT's spotting mechanism](http://wiki.wargaming.net/en/Battle_Mechanics#Spotting_Mechanics)


# References:
## Tutorials:
- [Tom Looman's Outline effect](http://www.tomlooman.com/outline-effect-in-unreal-engine-4)
- [PawelM's Outline effect, with explanation on Sobel operator](http://www.michalorzelek.com/blog/tutorial-creating-outline-effect-around-objects)

## Materials:
- Sixthsense/Newtype flash SFX and Char Aznable image are owned by Sunrise Inc. and Bandai Inc.
- Epic's Vehicle Game demo: You can find it under "Learn" subtab of Epic Game Launcher - Unreal Engine Tab.
- [Main BGM: Matthew Pablo - Dream Raid I](https://opengameart.org/content/dream-raid-cinematic-action-soundtrack)
- [Reload SFXs - Drawer SFXs - Mafon2](https://freesound.org/people/Mafon2/sounds/330021)
