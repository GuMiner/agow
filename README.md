# agow
## About
--------

An experiment in fragment shading, open world simulation, and style generation.

![FinStat1.png][finstat]
[finstat]:./FinStat1.PNG

![FinStat1.png][finstat2]
[finstat2]:./FinStat2.PNG

![FinStat1.png][finstat3]
[finstat3]:./FinStat3.PNG

![Status2.png][status2]
[status2]:./Status2.PNG

## Status
---------

Agow is on indefinite hold, as I've learned enough to move onto more feasible projects.

I started developing agow with a compelling concept, that unfortunately didn't pan out once I started writing the story. 
Like [TemperFine] (https://github.com/GuMiner/TemperFine) I learned a significant amount, which for this project included game design, OpenGL performance, Bullet physics and GIS data systems. 

What did work:
* Decoding King County into a series of 1000x1000 pixel heightmaps, breaking those heightmaps into smaller 100x100 heightmaps, and dynamically-loading the sub-heightmaps in real-time and no visible seams.
* Generating grass, trees, buildings, etc. from the type assigned to each heightmap grid pixel.
* Using external dependencies (GLM/STB/etc) to enable me to focus on the unique aspects of this project.
* Segmenting buildings for random height generation and interesting building demolitions.
* Event-based dialog, with an auto-paging dialog pane.
* Basic integration of real-time Bullet physics with OpenGL rendering.

What didn't work:
* **Dynamically changing a subset of items from being drawn dynamically to statically every frame.** Each building segment is initially drawn statically, however if it is hit with a collision that segment is added to a list of items whose matrices are sent to OpenGL every frame. While drawing *all* the building segments statically is efficient, and *some* of the building segments dynamically is also efficient, moving segments between these two lists is very inefficient.
* **Insufficient design**. The premise was compelling, but not feasible for a single-person prototype, much less a full game. While I leveraged common code between TemperFine and agow, I didn't create [GuCommon] (https://github.com/GuMiner/GuCommon) until much later. The AI wasn't fully thought out, as I was assuming behavioral imputs that were not easily achievable for the AI to use. My attempts at creating TODO lists were not comprehensive nor cohesive enough, which served as a good warning indicator that the premise would not lead to a feasible nor compelling story.
* **Bullet physics doesn't perform well with an immense number of physical objects (3D buildings) in realtime.** I attempted to work around this problem by assigning a ghost object to each building, that would spawn into building pieces when hit by another object. When that wasn't sufficient, I split the physics computation onto a separate thread. Unfortunately, that brought back the lag and interpolation problems that I had encountered previously writing this game, but in an unavoidable manner that became significantly worse when many physical collisions were happening simultaneously.

## Dependencies
---------------
Licenses for dependencies are within the *licenses* folder.

* [GLM 0.9.8.1] (http://glm.g-truc.net/0.9.8/index.html)
* [GLFW 3.2.1] (http://www.glfw.org/)
* [GLEW 1.12] (http://glew.sourceforge.net/)
* [SFML 2.3.2] (http://www.sfml-dev.org/index.php)
* [STB latest] (https://github.com/nothings/stb)
* [Bullet Physics 2.83.7] (https://github.com/bulletphysics/bullet3/releases/tag/2.83.7)
* [MESA 11.1.1] (http://www.mesa3d.org)
* [King County GIS Data Center] (http://www5.kingcounty.gov/gisdataportal/) (Data provided by permission of King County)
* [GLSL 2D Simplex Noise] (https://raw.githubusercontent.com/ashima/webgl-noise/master/src/noise2D.glsl)