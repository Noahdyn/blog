---
title: Stress Testing Data Oriented Design
date: 2025-07-01
header_img_src: path_finding_asset_1.svg
---

# Preface

Coming from the corporate software world one will definitely be familiar with the concept of Object Oriented Programming and Clean Code™. While some may like these concepts, my frustration with them has steadily grown over the years. Not only do they produce code that is ridden with abstractions, hard to change (this is controversial since many argue otherwise) and horrible in its [performance](https://www.youtube.com/watch?v=tD5NrevFtbU&ab_channel=MollyRocket).

Looking for different paradigm to structure my software I came to enjoy a very procedural way of programming (and still do!).
Eventually I stumbled upon this concept called Data Oriented Design. I will keep my explanation of DOD very brief so [here](https://www.youtube.com/watch?app=desktop&v=rX0ItVEVjHc&ab_channel=CppCon) and [here](https://www.dataorienteddesign.com/dodmain/) are my favourite resources to get deeper into it.

# Data Oriented Design

DOD is a different way of thinking about your data, the conventional way a programmer models their data in a program is by thinking about the requirements. He models the data in its context, so for a Game System he might create a Entity Class which contains a lot of information on each Object:

```java
class Entity {
	Vector3 position;
	Vector3 rotation;
	float mass;
	Vector3 velocity;
		
```

Instances of this class may get stored in one large Array Entities. This brings some problems with it (and we haven't even added inheritance yet!).
Iterating over Entities will load all these fields into memory each time, even though we may only need parts of it. 
Also this Array Of Structures approach leads to cache misses as you're loading large chunks of mostly irrelevant data.

So your CPU comes with these really fast pieces of memory, called caches, which get used to speed up memory access. Accessing these caches is faster by a significant magnitude than your regular memory (usually 2-10 nanoseconds vs around a 100 nanoseconds). Now when you iterate over an array your CPU prefetches a cache line (usually 64 bytes) with your data so that it can speed up access. If you now load up all this unnecessary data through large structs or objects your cache lines fill up rather fast, leading to cache misses, which mean you have to access your slower RAM on every iteration step.

In DOD you focus on how your data is used and transformed, grouping fields by access patterns to optimize for these cache behaviors. It also strips context from your data, making it easier to change and react to changes in business requirements.

```java
class SpatialData {
	Vector3[] positions;
	Vector3[] rotations;
}

class PhysicData {
	float[] masses;
	Vector3[] velocities;
}
```

Notice how did the data is not only laid out in a way that matches the way its accessed, we now also use something called Structure Of Arrays (SOA).

It turns out CPUs are quite good at working with contiguous data (more specifically arrays), by utilizing the cache in this way we get a notable performance improvement.

# The Odin Programming Language

While Data Oriented Design can be used in any programming language its effects might be less notable on garbage collected languages, since the GC can mess with your memory layout.

Odin, a relatively young programming language brings some convenience features for data oriented design. Most notably the #soa Data Structure:

```odin
N :: 2

struct SpatialData {
	Vector3 position;
	Vector3 rotation;
}

soa: #soa[N]Vector3

soa[0].position = {x=0, y=0, z=0}
soa[0].rotation = {x=0, y=0, z=0}

assert(soa[0].x == &soa.x[0])

```


This structure is very convenient as it lets you treat a Structure of Arrays like a regular Array of Structs while your data gets laid out as SOA in memory.

# Putting it to the test

When learning about new concepts I try to solidify my new knowledge by applying it in a small scoped project.
So to get the maximum value out of data oriented design I've been looking for a use case with big performance requirements. 
Eventually I decided on a Collision-Detection demo of a game in a bullet-hell style a la Vampire Survivors.

Rendering a lot of entities on the screen and checking for possible collisions between these entities all within a few milliseconds (around 16 for 60 fps) means you will have to value your codes performance. A lot. That's one of the reasons Vampire Survivors put a soft cap of 300 enemies into their code.

So I knew I'd have a player entity, bullets and enemies. But in the DOD spirit I didn't just create structs for each type but rather think about what my data actually is and how I am gonna access it. Eventually I came up with this data layout:

```odin

EntityType :: enum {
	PLAYER,
	BULLET,
	ENEMY,
}

RenderInfo :: struct {
	type:          EntityType,
	color:         rl.Color,
	size:          f32,
	to_be_removed: bool,
}

Movement :: struct {
	direction: rl.Vector2,
	speed:     f32,
}

...
	movement_soa: #soa[dynamic]Movement
	movement_soa = make(#soa[dynamic]Movement, MAX_ENTITIES)
	defer delete(movement_soa)

	position_soa: #soa[dynamic]rl.Vector2
	position_soa = make(#soa[dynamic]rl.Vector2, MAX_ENTITIES)
	defer delete(position_soa)

	render_soa: #soa[dynamic]RenderInfo
	render_soa = make(#soa[dynamic]RenderInfo, MAX_ENTITIES)
	defer delete(render_soa)
...

```

Each entity is represented by its index in the different SOA's, I then can iterate over the needed soa fields whenever needed:

```odin
//update enemy movement to approach player
for i := 1; i < current_no_entities; i += 1 {
	if render_soa[i].type == EntityType.ENEMY {
		dir := position_soa[0] - position_soa[i]
		movement_soa[i].direction = rl.Vector2Normalize(dir)
	}
}

//update all positions based on their movement
for i := 0; i < current_no_entities; i += 1 {
	position_soa[i].x += movement_soa[i].direction.x * movement_soa[i].speed * dt
	position_soa[i].y += movement_soa[i].direction.y * movement_soa[i].speed * dt
}
```

For checking collisions I map the entity positions to a spatial grid, for each cell I check an entity against each entity in the same grid cell, as well as entities in potentially overlapping neighbouring cells.
This leads to potentially deeply nested loops, which may seem slow and scary at first, but since we got great cache locality and the knowledge that each cell will only contain relatively few entities on its own, this approach will still outperform most other algorithms & data structures (for our specific scenario at least).

# Conclusion

![Demo Video of a data oriented bullet hell implementation](dod_bullets.mp4)

My demo remains a steady 60 FPS up until around 250 THOUSAND entities on screen, remaining over 40 fps up until over 300k entities.
This number is way higher than I would've anticipated and a lot higher than the caps of similar games that I've seen (of course these games do a lot more than my little demo does).

I will definitely keep Data Oriented Design in mind for future projects, and apply whats reasonable in other projects, even if they do not follow a strict DOD approach. I really like the approach of understanding your data first and staying flexible for changes in requirements, making DOD not solely useful for performance optimizations.

The source code of my demo can be found [here](https://github.com/Noahdyn/dod_bullet_hell/blob/main/game.odin)

