package com.example.androidtinygl;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.example.androidtinygl.databinding.ActivityMainBinding;

import java.nio.ByteBuffer;

public class MainActivity extends AppCompatActivity {
    private Bitmap _bitmap;

    // Used to load the 'androidtinygl' library on application startup.
    static {
        System.loadLibrary("androidtinygl");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        nativeSetContext(this, this.getAssets());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());

        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        int h = displayMetrics.heightPixels;
        int w = displayMetrics.widthPixels;
        w = 640;
        h = 480;
        Bitmap.Config config = Bitmap.Config.ARGB_8888;
        _bitmap = Bitmap.createBitmap(w, h, config);

        ImageView imageView = binding.imageView;
        imageView.setImageBitmap(_bitmap);

        nativeSetBitmap(_bitmap);

        initGL(w, h);

        Handler handler = new Handler();
        Runnable run = new Runnable() {
            @Override
            public void run() {
                drawGL();

                nativeSyncBitmapData(_bitmap);
                // redraw imageview: https://stackoverflow.com/a/11894378
                imageView.postInvalidate();

//                Log.e("##", getImageBounds(imageView).toString());

//                Bitmap bitmap = nativeGetBitmapFromBitmapData();
//                imageView.setImageBitmap(bitmap);

                handler.postDelayed(this, 1000/60);
            }
        };
        handler.post(run);

        imageView.setOnTouchListener(new View.OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                RectF bound = getImageBounds(imageView);
                if (!bound.contains(event.getX(), event.getY())) {
                    Log.e("##", "not int bitmap");
                    return false;
                }

                int action = event.getActionMasked();


                float x = event.getX()/bound.width();
                float y = (event.getY()-bound.top)/bound.height();

                switch(action) {
                case MotionEvent.ACTION_DOWN:
                    nativeTouchesBegin(x, y);
                    break;
                case MotionEvent.ACTION_MOVE:
                    nativeTouchesMove(x, y);
                    break;
                case MotionEvent.ACTION_UP:
                    nativeTouchesEnd(x, y);
                    break;
                case MotionEvent.ACTION_CANCEL:
                    break;
            }
                return true;
            }
        });
    }

    @Override
    protected void onPause()
    {
        super.onPause();
    }
    @Override
    protected void onResume()
    {
        super.onResume();
    }

    /**
     * Helper method to get the bounds of image inside the imageView.
     *
     * @param imageView the imageView.
     * @return bounding rectangle of the image.
     */
    public static RectF getImageBounds(ImageView imageView) {
        RectF bounds = new RectF();
        Drawable drawable = imageView.getDrawable();
        if (drawable != null) {
            imageView.getImageMatrix().mapRect(bounds, new RectF(drawable.getBounds()));
        }
        return bounds;
    }
    /**
     * A native method that is implemented by the 'androidtinygl' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public static native void nativeSetContext(final Context pContext, final AssetManager pAssetManager);
    public static native void initGL(int w, int h);
    public static native void drawGL();
    public static native void nativeSetBitmap(Bitmap bitmap);
    public static native Bitmap nativeGetBitmapFromBitmapData();
    public static native void nativeSyncBitmapData(Bitmap bitmap);

    private static native void nativeTouchesBegin(final float pX, final float pY);
    private static native void nativeTouchesEnd(final float pX, final float pY);
    private static native void nativeTouchesMove(final float pXs, final float pYs);
//    private static native void nativeTouchesCancel(final float pXs, final float pYs);
}