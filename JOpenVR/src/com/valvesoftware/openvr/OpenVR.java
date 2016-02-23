package com.valvesoftware.openvr;

import com.valvesoftware.openvr.structs.*;

import java.io.IOException;
import java.io.InputStream;
import java.text.DecimalFormat;
import java.util.Properties;

public class OpenVR //implements IVR
{
	private boolean initialized = false;

    private ErrorInfo _lastErrorInfo = new ErrorInfo();

    public static String NOT_INITIALISED = "Not initialised";
    public String _initSummary = NOT_INITIALISED;
    private static String jopenvrVersion = null;

	private static boolean libraryLoaded = false;
	
	public OpenVR()
    {
        resetHMDInfo();
	}

    private void resetHMDInfo()
    {

    }

    private void resetTrackerInfo()
    {

    }

	public String getInitializationStatus()
	{
		return _initSummary;
	}

    public String getLastError()
    {
        return _getLastError().errorStr;
    }

    public static String getVersionString()
    {
        return "Unknown";
    }

    public static String getJOpenVRVersionString()
    {
        if (jopenvrVersion == null)
        {
            String path = "/META-INF/maven/com.valvesoftware.openvr/JOpenVR/pom.properties";
            InputStream stream = OpenVR.class.getResourceAsStream(path);
            if (stream == null)
                jopenvrVersion = "UNKNOWN";
            else {
                Properties props = new Properties();
                try {
                    props.load(stream);
                    stream.close();
                    jopenvrVersion = (String) props.get("version");
                } catch (IOException e) {
                    e.printStackTrace();
                    jopenvrVersion = "UNKNOWN";
                }
            }
        }

        return jopenvrVersion;
    }

	public boolean init()
    {
        OpenVR.LoadLibrary();

        if( !libraryLoaded )
        {
            _initSummary = "Load library failed";
            return false;
        }

        initialized = _initSubsystem();
        if( !initialized )
        {
            ErrorInfo error = _getLastError();
            _initSummary = error.errorStr;
            resetHMDInfo();
            return false;
        }

        //hmdParameters = _getHmdParameters();
        _initSummary = "OK";
		
		return initialized;
	}
	
	public boolean isInitialized()
    {
		return initialized;
	}

    public void destroy()
    {
        if (initialized)
        {
            _destroySubsystem();
        }

        _initSummary = NOT_INITIALISED;
        initialized = false;
    }


    // Native declarations

	protected native boolean         _initSubsystem();
    protected native void            _destroySubsystem();
    protected native ErrorInfo       _getLastError();

    public static void LoadLibrary()
    {
        if( libraryLoaded ) return;
        String os = System.getProperty("os.name");
        boolean is64bit = System.getProperty("sun.arch.data.model").equalsIgnoreCase("64");

        //Launcher takes care of extracting natives
        if( is64bit )
        {
            System.loadLibrary("openvr_api");
            System.loadLibrary("JOpenVRLibrary64");
            System.out.println("Loaded JOpenVR native library (64bit)");
        }
        else
        {
            System.loadLibrary("openvr_api");
            System.loadLibrary("JOpenVRLibrary");
            System.out.println("Loaded JOpenVR native library (32bit)");
        }

        libraryLoaded = true;
    }

    public static void main(String[] args)
    {
        long frameIndex = 0;
        System.out.println("JOpenVR version: " + OpenVR.getJOpenVRVersionString());

        // Will need to add the natives dir to your Java VM args: -Djava.library.path="<path to natives dir>"

        // Load the JRift library
        OpenVR.LoadLibrary();
        OpenVR ovr = new OpenVR();

        // Initialise
        if (!ovr.init())
        {
            System.out.println("Failed to initialise OpenVR lib: " + ovr.getLastError());
            return;
        }

        /*
        // Get the HMD information
        HmdParameters hmdDesc = or.getHmdParameters();
        System.out.println(hmdDesc.toString());

        // Determine render target size based on recommended sizes calculated by the Oculus SDK
        RenderTextureInfo renderTextureSize = or.getRenderTextureSizes(hmdDesc.DefaultEyeFov[0], hmdDesc.DefaultEyeFov[1], 1.0f);
        System.out.println("Render target size: " + renderTextureSize.CombinedTextureResolution.w + "x" + renderTextureSize.CombinedTextureResolution.h);

        // Setup render parameters
        GLConfig glConfig = new GLConfig();

        // TODO: Update!

        while (or.isInitialized())
        {
            ///frameIndex++;

            // Get tracker and eye pose information before beginFrame - if rendering configured
            FullPoseState trackedPoses = or.getTrackedPoses(frameIndex);
            Posef leyePose = trackedPoses.leftEyePose;
            Posef reyePose = trackedPoses.rightEyePose;

            // If you need a Quatf to Euler conversion...
            EulerOrient Leuler = or.getEulerAnglesDeg(leyePose.Orientation.inverted(),
                    1.0f,
                    Axis.Axis_Y,
                    Axis.Axis_X,
                    Axis.Axis_Z,
                    HandedSystem.Handed_L,
                    RotateDirection.Rotate_CCW);
            EulerOrient Reuler = or.getEulerAnglesDeg(reyePose.Orientation.inverted(),
                    1.0f,
                    Axis.Axis_Y,
                    Axis.Axis_X,
                    Axis.Axis_Z,
                    HandedSystem.Handed_L,
                    RotateDirection.Rotate_CCW);

            Vector3f Lpos = trackedPoses.leftEyePose.Position;
            Vector3f Rpos = trackedPoses.rightEyePose.Position;

            // In game render loop, would call (needed to have called configureRendering first):
            //or._beginFrame(frameIndex);

            //EyeType firstEyeToRender = hmdParameters.EyeRenderOrder[0];

            // Pseudo code
            //<"RenderEye(eyePoses.getEyePose(firstEyeToRender))">     (should be <5ms for DK2)

            //EyeType secondEyeToRender = hmdParameters.EyeRenderOrder[1];

            // Pseudo code
            //<"RenderEye(eyePoses.getEyePose(secondEyeToRender))">     (should be <5ms for DK2)

            //or.endFrame();

            try {

                System.out.println("\n");
                dumpPose("L Eye: ", Leuler, Lpos);
                dumpPose("R Eye: ", Reuler, Rpos);

                // Obviously you wouldn't sleep in your normal poll / render loop! You'd poll every frame.
                Thread.sleep(1000);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        */

        ovr.destroy();
    }

    /*
    public static void dumpPose(String prefix, EulerOrient euler, Vector3f pos)
    {
        DecimalFormat fmt = new DecimalFormat("+#,000.0000;-#");
        System.out.println(prefix + "Yaw: " + fmt.format(euler.yaw) + "\u0176 Pitch: " + fmt.format(euler.pitch) + "\u0176 Roll: " + fmt.format(euler.roll) + "\u0176" +
                " PosX: " + fmt.format(pos.x) + "m PosY: " + fmt.format(pos.y) + "m PosZ: " + fmt.format(pos.z) + "m");
    }
    */
}
