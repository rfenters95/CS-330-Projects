# FMU-Robotics-Projects

## About

Projects completed for CS 330: Robotics at Francis Marion University during Spring 2017. For each project I have listed the goals required to complete the assignment along with the code used to acheive these goals. Individual projects are available to download via [DownGit](https://minhaskamal.github.io/DownGit/#/home) in the links below.

## Project-1

### Project-1 Goals:
1. Cycle the power button from red to green in 16 even steps. Show each<br>
color for 1 second.

2. If the left bumper is pressed, light the robot's Check Robot LED while<br>
the bumper is depressed.

3. If the right bumper is pressed, light the robot's Debris LED while the<br>
bumper is depressed.

[Download Project-1](https://minhaskamal.github.io/DownGit/#/home?url=https://github.com/rfenters95/FMU-Robotics-Projects/tree/master/Project-1)

## Project-2

### Project-2 Goals:
1. Read the **Wall Signal** Sensor, and map it's value to the __Power LED Color__ value
of the **LED** command. <br> **Hint: mapping will not be 1-to-1**.

2. Read the **Bump** Sensor. Given the response respond as follows:
- If **both** sensors are pressed, then drive straight backwards until sensors are deactivated.
- If **one** sensor is pressed, then drive away from the activated sensor with an **ICC = 1m** until <br>
the sensor is deactivated.

[Download Project-2](https://minhaskamal.github.io/DownGit/#/home?url=https://github.com/rfenters95/FMU-Robotics-Projects/tree/master/Project-2)

## Project-3

### Project-3 Goals:
1. Drive in a square pattern. With side length = 1m.

2. If the bumped, stop and wait 1/10 second; repeat until bump is <br>
no longer detected. **Delay should not effect distance Roomba must travel!**

[Download Project-3](https://minhaskamal.github.io/DownGit/#/home?url=https://github.com/rfenters95/FMU-Robotics-Projects/tree/master/Project-3)

## Project-4

### Project-4 Goals:
1. Start with Roomba some distance from a wall. <br>
Have the Roomba drive into (bump) and correct itself.

2. Follow the wall maintaining a distance **X**.

3. Should be able to handle a concave or convex corner.

[Download Project-4](https://minhaskamal.github.io/DownGit/#/home?url=https://github.com/rfenters95/FMU-Robotics-Projects/tree/master/Project-4)

## Project-5

### Project-5 Goals:
Roomba will start in the center of a square **(side length = 4ft)**. <br>
It's job will be to find **X** notecards will be placed randomly throughout the square. <br>
Discovery of a notecard will be **indicated by flashing an LED(s) or playing a tone**. <br>
Upon discovery of all **X** notecards, stop the Roomba and play a song.

[Download Project-5](https://minhaskamal.github.io/DownGit/#/home?url=https://github.com/rfenters95/FMU-Robotics-Projects/tree/master/Project-5)
