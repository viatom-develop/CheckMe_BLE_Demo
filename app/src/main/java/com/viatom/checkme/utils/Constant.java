package com.viatom.checkme.utils;

import android.content.Context;

import com.viatom.checkme.R;

import java.io.File;

public class Constant {
    public final static int[] ICO_IMG = new int[]{R.drawable.ico1,
            R.drawable.ico2, R.drawable.ico3, R.drawable.ico4, R.drawable.ico5,
            R.drawable.ico6, R.drawable.ico7, R.drawable.ico8, R.drawable.ico9,
            R.drawable.ico10};

    public final static int[] RESULT_IMG = { R.drawable.smile, R.drawable.cry,
            R.drawable.none };
    public static String filePath;

    public static String getPathX(String s) {
        return filePath + s;
    }

    public static void initVar(Context context) {
        File[] fs = context.getExternalFilesDirs(null);
        if (fs != null && fs.length >= 1) {
            filePath = fs[0].getAbsolutePath() + "/";
        }
    }
}
