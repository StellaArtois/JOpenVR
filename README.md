JOpenVR
=======

Java Wrapper (JNI) for OpenVR. 


Building
========

- Ensure the JAVA_HOME environment variable is set-up and points to a valid Java installation directory. I currently build with JDK 1.6.0.45 64bit.
- Install CMake >= V2.8
- Install Maven if you want to be able to easily package and deploy JOpenVR versions.

JOpenVR - the java part
---------------------

- Change directory to `<JOpenVR root>/JOpenVR/`
- Run `mvn package`. This will create the appropriately named <JOpenVR-version>.jar in `<JOpenVR root>/JOpenVR/target/`

JOpenVRLibrary - the C++ JNI part
-------------------------------

- Change directory to `<JOpenVR root>/JOpenVRLibrary`
- clone Valve’s OpenVR Git repository from [github.com/ValveSoftware/openvr](https://github.com/ValveSoftware/openvr) to `<JOpenVR root/JOpenVRLibrary/openvr`
- run the build script appropriate for your platform (either `build_osx.sh`, `build_linux.sh` or `build_windows.bat`)
- run `mvn package`. This will create the JARs containing the native libraries in `<JOpenVR root/JOpenVRLibrary/target`.



