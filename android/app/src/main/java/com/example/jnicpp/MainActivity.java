package com.example.jnicpp;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.VibratorManager;
import android.util.Log;
import android.view.MotionEvent;
import android.view.WindowManager;
import android.widget.Toast;
import com.example.jnicpp.databinding.ActivityMainBinding;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.content.pm.ActivityInfo;

import java.util.concurrent.atomic.AtomicBoolean;
import android.os.Build;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.content.Context;

public class MainActivity extends AppCompatActivity implements GLSurfaceView.Renderer {
    private static final String TAG = "MainActivity";
    // Used to load the 'jnicpp' library on application startup.
    static {
        try {
            System.loadLibrary("fmod");
            System.loadLibrary("jnicpp");

            Log.d(TAG, "Native library loaded successfully");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load native library", e);
        }
    }
    private ActivityMainBinding binding;
    private GLSurfaceView glSurfaceView;
    //private boolean isInitialized = false;
    private AtomicBoolean isInitialized = new AtomicBoolean(false);

    private static Vibrator vibrator;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d("JNI_DEBUG", this.getClass().getName());
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) { // Android 12 (API 31) or above
            VibratorManager vibratorManager = (VibratorManager) getSystemService(Context.VIBRATOR_MANAGER_SERVICE);
            vibrator = vibratorManager.getDefaultVibrator();
        } else {
            // Backward compatibility for older Android versions
            vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        }
        Log.d(TAG, "onCreate called");
        try {
            binding = ActivityMainBinding.inflate(getLayoutInflater());
            setContentView(binding.getRoot());
// Setup GLSurfaceView
            glSurfaceView = binding.glSurfaceView;
            glSurfaceView.setEGLContextClientVersion(3); // OpenGL ES 3.0
            glSurfaceView.setPreserveEGLContextOnPause(true);
            glSurfaceView.setRenderer(this);
            glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
            glSurfaceView.setOnTouchListener((v, event) -> {
                // Forward to native
                int action = event.getActionMasked();

                if (action == MotionEvent.ACTION_MOVE) {
                    for (int i = 0; i < event.getPointerCount(); i++) {
                        int pointerId = event.getPointerId(i);
                        float x = event.getX(i);
                        float y = event.getY(i);

                        nativeOnTouch(action, pointerId, x, y);
                    }
                } else {
                    // DOWN/UP → only report the finger that changed
                    int index = event.getActionIndex();
                    int pointerId = event.getPointerId(index);
                    float x = event.getX(index);
                    float y = event.getY(index);

                    nativeOnTouch(action, pointerId, x, y);
                }

                return  true;
            });
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);

            WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView()).hide(WindowInsetsCompat.Type.systemBars());
            nativeInitClassLoader(getClassLoader());
// Example of a call to a native method
            String message = stringFromJNI();
            Log.d(TAG, "Native message: " + message);
        } catch (Exception e) {
            Log.e(TAG, "Error in onCreate", e);
            showToast("Error initializing app: " + e.getMessage());
        }

        getOnBackPressedDispatcher().addCallback(this, new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                Log.d(TAG, "Back pressed (gesture or button), cleaning up and exiting");
                if (isInitialized.get()) {
                    Log.d(TAG, "HELLO, TRYING TO CLEAN UP OPENGL");
                    try {
                        cleanupGL();    // call into your native cleanup
                        isInitialized.set(false);
                    } catch (Exception e) {
                        Log.e(TAG, "Error during cleanup", e);
                    }
                }
                finishAffinity();  // close all activities
                System.exit(0);    // ensure process exit
            }
        });

    }
    @Override
    protected void onPause() {
        super.onPause();

        Log.d(TAG, "onPause called");
        if (glSurfaceView != null) {
            glSurfaceView.onPause();
        }
        if(isInitialized.get())
            focusChanged(false);
    }
    @Override
    protected void onResume() {
        super.onResume();

        Log.d(TAG, "onResume called");
        if (glSurfaceView != null) {
            glSurfaceView.onResume();
        }
        if(isInitialized.get())
            focusChanged(true);
        WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView()).hide(WindowInsetsCompat.Type.systemBars());
    }
    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy called");
        if (isInitialized.get()) {
            Log.d(TAG, "HELLO, TRYING TO CLEAN UP OPENGL");
            try {
                cleanupGL();
                isInitialized.set(false);// = false;
            } catch (Exception e) {
                Log.e(TAG, "Error during cleanup", e);
            }
        }
    }

    // Helper method to show toast from any thread
    private void showToast(final String message) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this, message, Toast.LENGTH_SHORT).show();
            }
        });
    }
    // GLSurfaceView.Renderer implementation
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.d(TAG, "Surface created");
        try {
            boolean success = initGL(getAssets());
            if(success) {


                Log.d(TAG, "openGL initialized successfully");
//                initIMGUI();
                Log.d(TAG, "ImGui initialized successfully");
                isInitialized.set(true);
            }
        } catch(Exception e) {
            Log.e(TAG, "Error in onSurfaceCreated", e);
            showToast("Error creating surface: " + e.getMessage());
        }
    }
    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        Log.d(TAG, "Surface changed: " + width + "x" + height);
        screenSizeChanged(width, height);
// Viewport is handled in native code
    }

    public static void vibrate(int milliseconds,int amplitude)
    {
        if (vibrator != null && vibrator.hasVibrator()) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                amplitude = Math.max(1, Math.min(amplitude, 255)); // clamp

                vibrator.vibrate(
                        VibrationEffect.createOneShot(
                                milliseconds,
                                amplitude
                        )
                );
            } else {
                // Old devices → no intensity control
                vibrator.vibrate(milliseconds);
            }
        }
    }
    @Override
    public void onDrawFrame(GL10 gl) {

        if (isInitialized.get()) {
            try {
                renderFrame();
            } catch (Exception e) {
                Log.e(TAG, "Error in onDrawFrame", e);
            }
        }

        if(killAppJNI())
        {
            QuitApp();
        }
    }

    public void QuitApp() {
        finishAffinity();
        System.exit(0);
    }
    /**
     * A native method that is implemented by the 'jnicpp' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    /**
     * Initialize OpenGL ES 3.0 renderer
     */
    public native boolean initGL(android.content.res.AssetManager assetManager);
    /**
     * Render a frame
     */
    public native void renderFrame();
    /**
     * screen size changed
     */
    public native void screenSizeChanged(int width, int height);
    /**
     * Cleanup OpenGL resources
     */
    public native void cleanupGL();

    public native void nativeOnTouch(int action, int pointerID ,float x, float y);
    public native void initIMGUI();

    /**
     * app focus changed
     */
    public native void focusChanged(boolean isFocused);

    /**
     * A native method that is implemented by the 'jnicpp' native library,
     * which is packaged with this application.
     */
    public native boolean killAppJNI();

    public native void nativeInitClassLoader(ClassLoader loader);

}
