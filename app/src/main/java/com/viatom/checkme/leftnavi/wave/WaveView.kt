package com.viatom.checkme.leftnavi.wave

import android.content.Context
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.Path
import android.util.AttributeSet
import android.view.View
import androidx.core.content.ContextCompat
import com.viatom.checkme.R

class WaveView : View {
    var canvas: Canvas? = null
    var data: IntArray? = null
    private val wavePaint = Paint()

    constructor(context: Context?) : super(context) {
        init()
    }

    constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs) {
        init()
    }

    constructor(context: Context?, attrs: AttributeSet?, defStyle: Int) : super(
        context,
        attrs,
        defStyle
    ) {
        init()
    }

    private fun init() {
        wavePaint.apply {
            color = getColor(R.color.white)
            style = Paint.Style.STROKE
            strokeWidth = 2.0f
        }

    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        this.canvas = canvas
        drawWave(canvas)
    }


    private fun drawWave(canvas: Canvas) {
        canvas.drawColor(getColor(R.color.black))
        data?.apply {
            val p = Path()
            p.moveTo(0f, height * 1 / 2 - this[0].toFloat()/5 )
            for (index in 0 until size) {
                val a = height * 1 / 2 - this[index].toFloat() /5
                if (a in 0.0..width.toDouble()) {
                    p.lineTo(
                        index.toFloat() * width / size,
                        a
                    )
                }

            }
            canvas.drawPath(p, wavePaint)
        }


    }

    private fun getColor(resource_id: Int): Int {
        return ContextCompat.getColor(context, resource_id)
    }
}