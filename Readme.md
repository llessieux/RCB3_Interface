# RCB3 Interface

This small project is aimed at developing a small DLL that will enable any C/C++ developer to communicate with an RCB3 driven robot.

With it, you will also able to control multiple robots in the event you have more than one connected (probably via bluetooth ).

I have made the choice to be very aggressive in terms of call validation, so all(well 95%) parameters will be checked and appropriate errors will be reported in case of a violation of the contract (Which is mainly the RCB3 Command references Japanese version).

Any exception caused by invalid pointers passed will also be caught and reported.

This project was done in 2007 and rather than letting it rot in my harddrive, I decided to move it to GitHub.
The code committed is the last version I wrote.
I am not actively working on the project anymore.

## History

### Version 1.00 First Release

    C API complete.
    All RCB3 documented functions implemented.
    Notification implemented too.
    Documentation (work in progress) covers most of the functions but it could with more details.
    Enable multiple RCB3 to be controlled from the same program.
    C++ sample to show usage
    C# and VB.NET wrappers provided but not really tested so far.
    C# Very Small Test application

### Version 1.01

I just realized after receiving an email about some bluetooth module that you might want to send commands to the module to activate some special mode.
These commands will not be processed by the RCB3 but might be essential for any wireless system.
So I have added 2 new functions enabling direct communication to the COM port.

### Version 1.02

Next feature coming is a native support of the BlueSmirf module to interface with the RCB3.
Well as soon as I figure why they don't talk to each other...

I'll also had a CreateDevice Function which is not doing the check after opening the com port, that way you will be able with the previously added function to do whatever you want.

At the moment, the creation might fail even before you had the chance to configure a potential wireless module.

It will also include a better support of notifications, at the moment collisions can occur between commands and notification resulting in a failure of the command. The new version will take care of that.


While the BlueSmirf interface seems to work, in my case I have too many errors to be able to use it properly.

The Sources are included in this package.

### Version 1.03

not released.

Add Raw functions for the Analog channels Set/Get functions. return/set the value directly as int instead of float (Volt).

### Version 1.04

Renamed 2 functions which had incorrect names. Set/GetScenarioPortUsage into Set/GetSerialServoPortUsage. Add loading of motions,scenario and home position from files.


### Version 1.05 (Last version)

Fix the VB wrapper. Honestly I had not tested it before so it was no surprise to me that it was not working in the first place.

I have included a small VB console application to test that it was at least compiling correctly.

Also fix some error message that were not reporting correctly what it was supposed to.

