This is the add-ons section. These are features that can improve the 
experience of the average user but don't fit with the tone of the rest
of the project. 

$ make   will compile all of the add-ons available. If you want a specfic
add-on, run   $ make <add-on>

color_picker
This addon will let you select the color of the dark and light squares on 
the chess board. 
Make sure your terminal has at least 25 lines vertically to display properly.
If you find yourself with fewer than 25 lines, Ctrl-C or moving the cursor to
the bottom right and pressing enter will quit without changing the colors. 
This add-on modifies the settings.h so for changes to take effect you must 
run   $ make clean   in your chessterm directory. 