# H2O

<!-- <img src="readme/h2o.gif" width="533" height="311"> -->
<img src="readme/h2o.gif" width="800" height="467">

Features:
* Automatic recalculation of optimal water consumption interval every second, immediately after 1 glass is due to be consumed
* Interval recalculation on early consumption (interval increase)
* Hibernation mode Start Time / End Time can be set in `h2o.db` file (obviously you won't consume H2O while sleeping, although...)
* Saves state in `h2o.db` file on each input. If you close the program, it will recalculate interval again based on already consumed amount
* No annoying sounds/alarms etc. Just silent flashing on your taskbar until you consume dat H2O!
* Double click protection
* RMB click minimises window

`h2o.db` format:
```
HH:mm ---> Start Time
HH:mm ---> End Time
GlASS ---> Amount of milliliters that fits in you H2O consumption vessel
H2OG  ---> H2O consumption goal for the day
H2OC  ---> H2O already consumed today
```
