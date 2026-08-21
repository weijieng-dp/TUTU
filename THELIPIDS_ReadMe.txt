GAM200-ENGINE
by THE LIPIDS
__________________________________________________________________________________________________________________
Team Members:
RTIS	JIANG Junbo	                                    j.junbo@digipen.edu
RTIS	NG Wei Jie	                                  weijie.ng@digipen.edu
RTIS	OU Yukang	                                  yukang.ou@digipen.edu
RTIS	TAN Jun Jie	                                   t.junjie@digipen.edu
IMGD	TAN Kaeden Jiawei	                   kaedenjiawei.tan@digipen.edu
IMGD	ZHANG Mingyang	                         mingyang.zhang@digipen.edu
UXGD	LORENZO YONGYONG De Guzman Adrian	 d.lorenzoyongoyong@digipen.edu
UXGD	ZHANG Yingjie	                              yingjie.z@digipen.edu
__________________________________________________________________________________________________________________
Team Roles (Primary | Secondary | Championing):
RTIS	JIANG Junbo	                         Programmer	       | Designer     |   Debugging Tools Champion
RTIS	NG Wei Jie	                         Technical Lead	   | Programmer   |   Engine Champion/Input Champion
RTIS	OU Yukang	                         Programmer	         | Designer     |   Graphics/Rendering Champion
RTIS	TAN Jun Jie	                         Programmer	       | Designer     |   Graphics/Rendering Champion
IMGD	TAN Kaeden Jiawei	                 Programmer	         | Designer     |   Level Editor Champion
IMGD	ZHANG Mingyang	                     Product Manager   | Programmer   |   Physics/Collision Champion
UXGD	LORENZO YONGYONG De Guzman Adrian	 Design Lead	       | Artist	      |   Level Design Champion
UXGD	ZHANG Yingjie	                     Art Lead	           | Audio Lead   |   Story Champion
__________________________________________________________________________________________________________________

Game Concept:

Get Me Out of Hell! is a top-down roguelike dungeon crawler for Android inspired by The Binding of Isaac and Blue 
Prince. Players control Usa, a moon rabbit cast into Hell, who must build and traverse her own dungeon to escape. 
Each round begins with a unique map-building phase where players drag and drop tiles to connect a start and end 
point, balancing combat, rest,and treasure rooms. Combat features auto-aimed projectile attacks tailored for 
mobile, while items modify stats, projectiles,and status effects to scale with difficulty. Enemies vary by Hell-
themed rounds and follow archetypes of small/fast, medium/ranged, or large/tank foes. Progression culminates in 
a boss fight, with a karma system influencing story outcomes. The game blends creepy-cute visuals, replayable 
dungeon runs, and strategic map-building to create a unique mobile roguelike experience.

__________________________________________________________________________________________________________________

Custom Demo Input and Usage:

Windows:
Title Screen:
 Click anywhere in game window to continue to main menu

Main Menu:
 Play                               - Opens Map Editor 
 Restart                            - Clears Save file, start a new run (currently non-functional)
 Quit Game                          - Exits application (non-functional in Debug build)
 Settings button                    - Opens settings for audio adjustment
 Question Mark button               - Opens tutorial page

Map Editor:
 Tile Selection Arrows              - Click left/right arrows to switch between tiles
 Play Button                        - Starts level

Test Level:
 WASD                               - Move
 Left Mouse Button                  - Shoot
 Esc                                - Pause 

Pause Menu:
 Resume Game                        - Resumes current level gameplay
 Home Button                        - Exit level, go back to main menu

 
Android:

Main Menu:
 Play                               - Opens Map Editor 
 Restart                            - Clears Save file, start a new run (currently non-funcitonal)
 Quit Game                          - Exits application
 Settings button                    - Opens settings for audio adjustment
 Question Mark button               - Opens tutorial page

Map Editor:
 Tile Selection Arrows              - Click left/right arrows to switch between tiles
 Play Button                        - Starts test level

Test Level:
 Left Joystick                      - Move
 Right Joystick                     - Shoot

Pause Menu:
 Resume Game                        - Resumes current level gameplay
 Home Button                        - Exit level, go back to main menu


__________________________________________________________________________________________________________________

Cheats
  
  NOTE: For testing and debug purposes only. 

  GOD MODE
  ----------------------------------
  Accesible on both Windows and Android:
  God mode button is hidden at top right of minimap
  - Take no damage + double damage

  Following cheats are Windows only.

  UNLOCK ALL CONTENT / ACHIEVEMENTS
  ----------------------------------
  Press the Tilde / Grave key ( ` ) on the Landing Page (First Screen).

  ITEM-SPECIFIC CHEATS
  ---------------------
  Hold Left Control + Right Control and press the corresponding number key.
  These cheats only work inside the dungeon.

  LCtrl + RCtrl + 1  ->  Poison
  LCtrl + RCtrl + 2  ->  Freeze
  LCtrl + RCtrl + 3  ->  Slow
  LCtrl + RCtrl + 4  ->  Homing
  LCtrl + RCtrl + 5  ->  Pierce
  LCtrl + RCtrl + 6  ->  Knockback
  LCtrl + RCtrl + 7  ->  Split
  LCtrl + RCtrl + 8  ->  Multishot
  LCtrl + RCtrl + 9  ->  Bounce

__________________________________________________________________________________________________________________

How To Play

  To begin the game, press the "Play" button on the Main Menu.

  MAP PHASE
  ---------
  - Add a tile from the right side of the screen to the map.
  - The tile must be connected to the start point in order to proceed.
  - You must clear the tile through combat before placing another tile.
  - Each tile has a specific enemy and/or punishment attached to it.
  - You may choose from 3 tiles per map phase.

  GAME PHASE
  ----------
  - Clear the placed tile by killing all enemies.
  - After clearing, choose one of the following rewards:
      > An item
      > A heal
      > Increased luck for the next tile
  - Return to the map phase anytime by clicking the book above the minimap.
  - Once all desired tiles are placed and you reach the end tile,
    jump into the dark hole.
  - Clear the final 3 waves of enemies to finish the run.

  ACHIEVEMENTS
  ------------
  - Certain in-game conditions unlock achievements.
  - Unlocking an achievement grants a reward, which may include:
      > More tile types
      > New enemy combinations
      > A new punishment
  - Rewards are granted after completing or losing a run once
    an achievement has been obtained.
