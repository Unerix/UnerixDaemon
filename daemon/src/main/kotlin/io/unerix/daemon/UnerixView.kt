package io.unerix.daemon

import android.content.Context
import android.graphics.SurfaceTexture
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.Surface
import android.view.TextureView

class UnerixView(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : TextureView(context, attrs, defStyleAttr), TextureView.SurfaceTextureListener {

    private var mDestroyed: Boolean = false
    private var mSurface: Surface? = null

    init {
        surfaceTextureListener = this
        isOpaque = false
        isFocusable = true
        keepScreenOn = true
        isFocusableInTouchMode = true
        onUnerixCreate()
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        if (mDestroyed) return false
        event?.let {
            when (it.action) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_MOVE -> {
                    onUnerixTouch(
                        touch = true,
                        x = it.x.toInt(),
                        y = it.y.toInt(),
                    )
                    return true
                }

                MotionEvent.ACTION_UP -> onUnerixTouch(
                    touch = false,
                    x = it.x.toInt(),
                    y = it.y.toInt(),
                )

                MotionEvent.ACTION_CANCEL -> onUnerixTouch(
                    touch = false,
                    x = it.x.toInt(),
                    y = it.y.toInt(),
                )
            }
        }
        if (event?.action == MotionEvent.ACTION_UP) {
            performClick()
        }
        return super.onTouchEvent(event)
    }

    override fun performClick(): Boolean {
        super.performClick()
        return false
    }

    override fun onSurfaceTextureAvailable(
        surface: SurfaceTexture,
        width: Int,
        height: Int
    ) {
        if (!mDestroyed) {
            mSurface = Surface(surface)
            onUnerixStartRender(mSurface)
        }
    }

    override fun onSurfaceTextureDestroyed(surface: SurfaceTexture): Boolean {
        if (!mDestroyed) {
            onUnerixStopRender()
        }
        if (mSurface != null) {
            mSurface?.release()
            mSurface = null
        }
        return false
    }

    override fun onSurfaceTextureSizeChanged(
        surface: SurfaceTexture,
        width: Int,
        height: Int
    ) {
        if (!mDestroyed && mSurface != null) {
            onUnerixStartRender(mSurface)
        }
    }

    override fun onSurfaceTextureUpdated(
        surface: SurfaceTexture,
    ) = Unit

    override fun onDetachedFromWindow() {
        if (!mDestroyed) {
            onUnerixDestroy()
            mDestroyed = true
        }
        super.onDetachedFromWindow()
    }

    private external fun onUnerixCreate()
    private external fun onUnerixStartRender(surface: Surface?)
    private external fun onUnerixTouch(touch: Boolean, x: Int, y: Int)
    private external fun onUnerixStopRender()
    private external fun onUnerixDestroy()

    companion object {
        init {
            System.loadLibrary("unerixd")
        }
    }
}