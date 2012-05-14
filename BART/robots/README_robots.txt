The main file for the robot scene is Robot.aff. It contains the camera
movements and all lights which consists of one point light representing
the sun and white ambient light representing background lighting. It 
also includes "city.aff" and 10 files "Robot1.aff" to Robot10.aff". 


All ten robot files contain an identical description of the robot hierarchy
with transforms and includes of solid robot part files. These robot part
files are:

ro-arm.aff      - upper right arm  
ru-arm.aff      - lower right arm
rg1.aff         - grip 1 (finger 1) on right hand
rg2.aff         - grip 2 (finger 2) on right hand
rg3.aff         - grip 3 (finger 3) on right hand
rg4.aff         - grip 4 (finger 4) on right hand
lo-arm.aff      - upper left arm
lu-arm.aff      - lower left arm
lo-leg.aff      - upper left leg
lu-leg.aff      - lower left leg
l-heel.aff      - left heel
l-middle.aff    - middle part of left foot
l-toes.aff      - toes of left foot
ro-leg.aff      - upper right leg
ru-leg.aff      - lower right leg
r-heel.aff      - right heel
r-middle.aff    - middle part of right foot
r-toes.aff      - toes of right foot
u-body.aff      - lower part of main body
led.aff         - middle joint of main body
mbody.aff       - middle part of main body
head.aff        - head

The 10 files "Robot1.aff" to "Robot10.aff" also contain individual 
key-frame parameters describing each robot's animations. There 
are 18 animated transforms:

gun_rot         - rotation of the robot gun
head_rot        - left/right rotation of the head 
body_lift       - rotation to raise/lower body
gun_lift        - rotation to raise/lower gun arm
robot_transl    - translation of robot position
step_transl     - wags the robot up and down for each step
robot_rot       - main rotation of robot direction
lo_leg_rot      - rotations for leg movements
lu_leg_rot      -       ''
l_heel_rot      -       ''
l_middle_rot    -       ''
l_toes_rot      -       ''
ro_leg_rot      -       ''
ru_leg_rot      -       ''
r_heel_rot      -       ''
r_middle_rot    -       ''
r_toes_rot      -       ''


"city.aff" contatins the whole environment which is a box with a sky 
as the roof and sky-scrapes with streets in between.

Colors that are in common for several objects are defined in the 
color files "color1.aff" to "color17.aff":

Color1.aff      - Color of street and some of the houses
Color2.aff      - Color of backside of neon lights etc
Color3.aff      - Color of pavement
Color4.aff      - Rocket bar and Table dance neon light colors
Color5.aff      - Color of Cat bar house, Sixx-house, Rocket-bar house and Scoopers Mendro-house
Color6.aff      - Color of Rocket bar-sign wall-holder
Color7.aff      - Glass of the street lights
Color8.aff      - Window frames and underside of roof of the colonnade at the Reteckers
Color9.aff      - Blue fancy thing up on Scoopeers Mendro house
Color10.aff     - Pillars of the Reteckers house
Color11.aff     - (unknown)
Color12.aff     - Glass of street lights outside Sixx-house
Color13.aff     - Windows of Reteckers
Color14.aff     - Color of pavement outside Reteckers
Color15.aff     - Windows of Reteckers
Color16.aff     - Relective color of robot, like cylinders
Color17.aff     - Black plastic rubber-color of robots

Total #polygons for each robot: 6,249

Total #polygons for the city.aff: 9,218


53 texture files are used:
corrugated.ppm
floor-1.ppm              
giveme.ppm
img0000.ppm
marble-2.ppm 
marble.ppm              
metal-wall-2.ppm         
mngirl6.ppm 
pavement.ppm
reteckers.ppm
rgb.ppm
road-straight-5.ppm
rock-3.ppm 
rocketbar.ppm
scoopeers.ppm 
sign-cat.ppm 
signracer.ppm 
signsixx.ppm 
soft_wall.ppm 
SPARK.ppm               
SPOT.ppm
texturer10.ppm 
texturer100.ppm 
texturer101.ppm 
texturer102.ppm          
texturer103.ppm 
texturer103_2.ppm 
texturer104.ppm 
texturer105.ppm 
texturer107.ppm 
texturer13.ppm 
texturer15.ppm 
texturer16.ppm 
texturer20.ppm 
texturer25.ppm 
texturer26.ppm          
texturer4.ppm 
texturer40.ppm 
texturer5.ppm            
texturer51.ppm 
texturer52.ppm 
texturer54.ppm 
texturer55.ppm 
texturer56.ppm 
texturer57.ppm 
texturer58.ppm 
texturer59.ppm 
texturer60.ppm 
texturer7.ppm 
texturer74.ppm 
texturer75.ppm 
texturer87.ppm 
zagurra.ppm
