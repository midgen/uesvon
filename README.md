# uesvon
Sparse Voxel Octree navigation plugin for UnrealEngine

Based on "3D Flight Navigation Using Sparse Voxel Octrees" by Daniel Brewer in Game AI Pro 3.

[Game AI Pro](https://www.gameaipro.com)

and his related GDC talk :

[GDC Vault - Getting Off The Navmesh](https://www.gdcvault.com/play/1022016/Getting-off-the-NavMesh-Navigating)

Status : Alpha. OCtree generation and pathfinding implemented as per the articles above. Not yet battle-tested, but a good basis for anyone to take further. Contributors welcome.

Engine Version : 4.21 (See branches for previous versions)

New - *Baking navigation data*

In the SVOVolume properties, there is a new Generation Strategy option.
* Generate OnBeginPlay - Rebuilds the navigation data when you start the game (not recommended)
* Use Baked - Generate navigation data in the editor, save, and load it when you start the game (recommended!)

Todo List :

* Add a separate BTTask and Action so you don't have to override MoveTo

Plugin is mostly working, just some edge cases where pathfinding can go wrong at the moment. Simple instructions :

* Install the plugin
* Create a new AIController from SVONAIController
* Add an SVONVolume to your scene
* Adjust the SVONVolume properties, enable some debug viz and click 'Generate' to check it
* On play, the SVONVolume will generate the octree (so you will get a pause with a large number of layers)
* Use the SVONAIController MoveTo (through BT if you want) to pathfind and follow the 3D path

[![UESVON Demo](http://img.youtube.com/vi/84AFdg0ykwY/0.jpg)](http://www.youtube.com/watch?v=84AFdg0ykwY "Video Title")


