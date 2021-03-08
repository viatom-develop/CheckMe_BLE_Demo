package com.viatom.checkme.leftnavi.dailyCheck

import android.content.Context
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.Path
import android.util.AttributeSet
import android.util.Log
import android.view.View
import com.viatom.checkme.R

class WaveView : View {
    var canvas: Canvas? = null
    var data: IntArray? = null
    val wave_paint = Paint()

    constructor(context: Context?) : super(context) {
        init(null, 0)
    }

    constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs) {
        init(attrs, 0)
    }

    constructor(context: Context?, attrs: AttributeSet?, defStyle: Int) : super(
        context,
        attrs,
        defStyle
    ) {
        init(attrs, defStyle)
    }

    private fun init(attrs: AttributeSet?, defStyle: Int) {
        wave_paint.color = getColor(R.color.white)
        wave_paint.style = Paint.Style.STROKE
        wave_paint.strokeWidth = 2.0f
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        Log.e("sdfds", "开始了的附近开了多少积分多少圣诞快乐积分   $width    $height")
        this.canvas = canvas
        drawWave(canvas)
    }


    private fun drawWave(canvas: Canvas) {
        canvas.drawColor(getColor(R.color.black))
        data?.apply {
            val p = Path()
            p.moveTo(0f, height * 1 / 2 - this[0].toFloat() / 1000)
            /* for((index,m) in this.withIndex()){
                 p.lineTo(index.toFloat()*width/size,height/4+m.toFloat()/500)
             }*/
            for (index in 0 until size) {
                p.lineTo(
                    index.toFloat() * width / size,
                    height * 1 / 2 - this[index].toFloat() / 1000
                )
            }
            canvas.drawPath(p, wave_paint)
        }


    }

    private fun getColor(resource_id: Int): Int {
        return resources.getColor(resource_id)
    }
}