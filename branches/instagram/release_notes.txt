Social Connect QML Plug-in
==========================

This plug-in delivers an interface for Qt Quick developers for accessing the
APIs of Facebook and Twitter that allow creating mobile applications powered
by social media.

The plug-in is hosted in Nokia Developer Projects:
- http://projects.developer.nokia.com/socialconnect

The main features of the plug-in are:
- Out-of-the-box support for Facebook and Twitter
- Integrated authentication implementation
- Simplified common interface for all supported services
- Interfaces for native API calls
- Design enabling easy addition of new services e.g. LinkedIn 

The plug-in also contains some simple examples.

The plug-in can be used as a pre-compiled shared library or you can copy the
source code into your project and compile it with your application. Although
the plug-in has been designed to be used with QML UI applications, it can also be
used as-is with Qt C++.

For more information on the implementation and usage, visit the wiki pages:
- http://projects.developer.nokia.com/socialconnect/wiki
- http://projects.developer.nokia.com/socialconnect/wiki/GettingStarted


1. Prerequisites
-------------------------------------------------------------------------------

 - Qt basics
 - Qt Quick basics


2. Project structure
-------------------------------------------------------------------------------

 |                  The root folder contains the project file, the license
 |                  information, and this file (release notes).
 |
 |- doc             Contains documentation on the project in general.
 |
 |- examples        Contains a set of example applications that utilise the
 |                  Social Connect plug-in.
 |
 |- plugin          Root folder of the plug-in implementation.
 |  |
 |  |- src          Plug-in source codes.
 |


3. Compatibility
-------------------------------------------------------------------------------

 - Symbian devices with Qt 4.7.4 and Qt Mobility 1.2.1 or higher.
 - Nokia N9 (MeeGo 1.2 Harmattan).

Developed with Qt SDK 1.2.1.

3.1 Required capabilities
-------------------------

In Symbian: ALL -TCB

For more information on Symbian capabilities, see:
- http://www.developer.nokia.com/Community/Wiki/App_types_that_require_sensitive_Symbian_capabilities


3.2 Known issues
----------------

None.


4. Building and using the plug-in
-------------------------------------------------------------------------------

4.1 Preparations
----------------

Check that you have the latest Qt SDK installed in the development environment
and the latest Qt version on the device.

4.2 Using the Qt SDK
--------------------

You can install and run the application on the device by using the Qt SDK.
Open the project in the SDK, set up the correct target (depending on the device
platform), and click the Compile button. For more details about this approach,
visit the Qt Getting Started section at Nokia Developer
(http://www.developer.nokia.com/Develop/Qt/Getting_started/).

4.3 Using the plug-in in your application
-----------------------------------------

The plug-in is by default compiled and installed to plugin/install. After this,
you can use the plug-in in your QML application by doing
"import SocialConnect 1.0". Note that the plugin/install directory must be in your
QML module import path.

Visit the online project documentation and see the examples for more
information.


5. License
-------------------------------------------------------------------------------

See the license text file delivered with this project. The license file is also
available online at
http://projects.developer.nokia.com/socialconnect/browser/trunk/Licence.txt


6. Version history
-------------------------------------------------------------------------------

1.0 Initial release
