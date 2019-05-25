// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation
import simd

class AnimatedFloat {

    var current: Float

    var delta: Float
    var steps = 1 { didSet { delta = (target - current) / Float(steps) } }
    var target: Float { didSet { delta = (target - current) / Float(steps) } }

    init(_ current: Float = 0.0, target: Float = 0.0, delta: Float = 0.0) {

        self.current = current
        self.target = target
        self.delta = delta
    }

    func animates() -> Bool {
        return current != target
    }

    func move() {

        if abs(current - target) < abs(delta) {
            current = target
        } else {
            current += delta
        }
    }
}

extension MetalView {
 
    // Returns true iff an animation is in progress
    func animates() -> Bool {

        return
            angleX.animates() ||
                currentYAngle != targetYAngle ||
                currentZAngle != targetZAngle ||
                currentEyeX != targetEyeX ||
                currentEyeY != targetEyeY ||
                currentEyeZ != targetEyeZ ||
                currentAlpha != targetAlpha
        /*
        return
            currentXAngle != targetXAngle ||
            currentYAngle != targetYAngle ||
            currentZAngle != targetZAngle ||
            currentEyeX != targetEyeX ||
            currentEyeY != targetEyeY ||
            currentEyeZ != targetEyeZ ||
            currentAlpha != targetAlpha
         */
    }

    func textureAnimates() -> Bool {

        return
            currentTexOriginX != targetTexOriginX ||
            currentTexOriginY != targetTexOriginY ||
            currentTexWidth != targetTexWidth ||
            currentTexHeight != targetTexHeight
    }
    
    func eyeX() -> Float {
        
        return currentEyeX
    }
    
    func setEyeX(_ newX: Float) {
        
        currentEyeX = newX
        targetEyeX = newX
        self.buildMatrices3D()
    }
    
    func eyeY() -> Float {
        
        return currentEyeY
    }
    
    func setEyeY(_ newY: Float) {
    
        currentEyeY = newY
        targetEyeY = newY
        self.buildMatrices3D()
    }
    
    func eyeZ() -> Float {
        
        return currentEyeZ
    }
    
    func setEyeZ(_ newZ: Float) {
    
        currentEyeZ = newZ
        targetEyeZ = newZ
        self.buildMatrices3D()
    }
    
    func updateAngles() {

        angleX.move()
        /*
        if abs(currentXAngle - targetXAngle) < abs(deltaXAngle) {
            currentXAngle = targetXAngle
        } else {
            currentXAngle += deltaXAngle
        }
        */

        if abs(currentYAngle - targetYAngle) < abs(deltaYAngle) {
            currentYAngle = targetYAngle
        } else {
            currentYAngle += deltaYAngle
        }
    
        if abs(currentZAngle - targetZAngle) < abs(deltaZAngle) {
            currentZAngle = targetZAngle
        } else {
            currentZAngle += deltaZAngle
        }
    
        if abs(currentEyeX - targetEyeX) < abs(deltaEyeX) {
            currentEyeX = targetEyeX
        } else {
            currentEyeX += deltaEyeX
        }
    
        if abs(currentEyeY - targetEyeY) < abs(deltaEyeY) {
            currentEyeY = targetEyeY
        } else {
            currentEyeY += deltaEyeY
        }
    
        if abs(currentEyeZ - targetEyeZ) < abs(deltaEyeZ) {
            currentEyeZ = targetEyeZ
        } else {
            currentEyeZ += deltaEyeZ
        }
    
        if abs(currentAlpha - targetAlpha) < abs(deltaAlpha) {
            currentAlpha = targetAlpha
        } else {
            currentAlpha += deltaAlpha
        }

        // DEPRECATED
        angleX.current -= (angleX.current >= 360) ? 360 : 0
        angleX.current += (angleX.current < 0) ? 360 : 0

        /*
        currentXAngle -= (currentXAngle >= 360.0) ? 360 : 0
        currentXAngle += (currentXAngle < 0.0) ? 360 : 0
        */
        currentYAngle -= (currentYAngle >= 360.0) ? 360 : 0
        currentYAngle += (currentYAngle < 0.0) ? 360 : 0
        currentZAngle -= (currentZAngle >= 360.0) ? 360 : 0
        currentZAngle += (currentZAngle < 0.0) ? 360 : 0
    }

    func updateTextureRect() {

        track("\(currentTexOriginX) \(targetTexOriginX) \(deltaTexOriginX)")
        if abs(currentTexOriginX - targetTexOriginX) < abs(deltaTexOriginX) {
            currentTexOriginX = targetTexOriginX
        } else {
            currentTexOriginX += deltaTexOriginX
        }

        track("\(currentTexOriginY) \(targetTexOriginY) \(deltaTexOriginY)")
        if abs(currentTexOriginY - targetTexOriginY) < abs(deltaTexOriginY) {
            currentTexOriginY = targetTexOriginY
        } else {
            currentTexOriginY += deltaTexOriginY
        }

        if abs(currentTexWidth - targetTexWidth) < abs(deltaTexWidth) {
            currentTexWidth = targetTexWidth
        } else {
            currentTexWidth += deltaTexWidth
        }

        if abs(currentTexHeight - targetTexHeight) < abs(deltaTexHeight) {
            currentTexHeight = targetTexHeight
        } else {
            currentTexHeight += deltaTexHeight
        }
/*
        textureRect.origin.x = currentTexOriginX
        textureRect.origin.y = currentTexOriginY
        textureRect.size.width = currentTexWidth
        textureRect.size.height = currentTexHeight
*/
        updateScreenGeometry()
    }
    
    func computeAnimationDeltaSteps(animationCycles: Int) {
    
        let cycles = Float(animationCycles)
        // deltaXAngle = (targetXAngle - currentXAngle) / cycles
        deltaYAngle = (targetYAngle - currentYAngle) / cycles
        deltaZAngle = (targetZAngle - currentZAngle) / cycles
        deltaEyeX = (targetEyeX - currentEyeX) / cycles
        deltaEyeY = (targetEyeY - currentEyeY) / cycles
        deltaEyeZ = (targetEyeZ - currentEyeZ) / cycles
        deltaAlpha = (targetAlpha - currentAlpha) / cycles
    }

    func computeTextureDeltaSteps(animationCycles: Int) {

        let cycles = CGFloat(animationCycles)
        deltaTexOriginX = (targetTexOriginX - currentTexOriginX) / cycles
        deltaTexOriginY = (targetTexOriginY - currentTexOriginY) / cycles
        deltaTexWidth = (targetTexWidth - currentTexWidth) / cycles
        deltaTexHeight = (targetTexHeight - currentTexHeight) / cycles
    }

    func zoomTextureIn(cycles: Int = 60) {

        track("Zooming texture in...")

        targetTexOriginX = 0.0
        targetTexOriginY = 0.0
        targetTexWidth = 728.0 / 768.0
        targetTexHeight = 286.0 / 288.0

        self.computeTextureDeltaSteps(animationCycles: cycles)
    }

    func zoomTextureOut(cycles: Int = 60) {

        track("Zooming texture out...")

        targetTexOriginX = 0.0
        targetTexOriginY = 0.0
        targetTexWidth = 1.0
        targetTexHeight = 1.0

        self.computeTextureDeltaSteps(animationCycles: cycles)
    }

    func zoom() {
    
        track("Zooming in...")
    
        currentEyeZ  = 6
        angleX.target = 0.0; angleX.steps = 120
            // targetXAngle = 0
        targetYAngle = 0
        targetZAngle = 0
    
        self.computeAnimationDeltaSteps(animationCycles: 120 /* 2 sec */)
    }
    
    func rotateBack() {
    
        track("Rotating back...")
    
        // targetXAngle = 0
        angleX.target = 0.0; angleX.steps = 60
        targetZAngle = 0
        targetYAngle += 90

        self.computeAnimationDeltaSteps(animationCycles: 60 /* 1 sec */)
  
        targetYAngle -= (targetYAngle >= 360) ? 360 : 0
    }
    
    func rotate() {
    
        track("Rotating...")
    
        // targetXAngle = 0
        angleX.target = 0.0; angleX.steps = 60
        targetZAngle = 0
        targetYAngle -= 90
    
        self.computeAnimationDeltaSteps(animationCycles: 60 /* 1 sec */)
        
        targetYAngle += (targetYAngle < 0) ? 360 : 0
    }
    
    func scroll() {
        
        track("Scrolling...")
    
        currentEyeY = -1.5
        // targetXAngle = 0
        angleX.target = 0.0; angleX.steps = 60
        targetYAngle = 0
        targetZAngle = 0
    
        self.computeAnimationDeltaSteps(animationCycles: 120 /* 2 sec */)
    }
    
    /*
    public func fadeIn() {
    
        track("Fading in...")
    
        currentXAngle = -90
        currentEyeZ   = 5.0
        currentEyeY   = 4.5
        targetXAngle  = 0
        targetYAngle  = 0
        targetZAngle  = 0
    
        self.computeAnimationDeltaSteps(animationCycles: 120 /* 2 sec */)
    }
    */
    
    func blendIn() {
        
        track("Blending in...")
        
        // targetXAngle = 0
        angleX.target = 0.0; angleX.steps = 10
        targetYAngle = 0
        targetZAngle = 0
        targetAlpha  = 1.0
        
        computeAnimationDeltaSteps(animationCycles: 10)
        // computeAnimationDeltaSteps(animationCycles: 1 /* almost immediately */)
    }

    func blendOut() {
        
        track("Blending out...")
        
        // targetXAngle = 0
        angleX.target = 0.0; angleX.steps = 40
        targetYAngle = 0
        targetZAngle = 0
        targetAlpha  = 0.0
        
        computeAnimationDeltaSteps(animationCycles: 40)
        // computeAnimationDeltaSteps(animationCycles: 15 /* 0.25 sec */)
    }
    
    func snapToFront() {
        
        track("Snapping to front...")
        
        currentEyeZ   = -0.05
        
        self.computeAnimationDeltaSteps(animationCycles: 15 /* 0.25 sec */)
    }
    
    //
    // Matrix utilities
    //
    
    func matrix_from_perspective(fovY: Float,
                                 aspect: Float,
                                 nearZ: Float,
                                 farZ: Float) -> matrix_float4x4 {
        
        // Variant 1: Keeps correct aspect ratio independent of window size
        let yscale = 1.0 / tanf(fovY * 0.5) // 1 / tan == cot
        let xscale = yscale / aspect
        let q = farZ / (farZ - nearZ)
    
        // Alternative: Adjust to window size
        // float yscale = 1.0f / tanf(fovY * 0.5f);
        // float xscale = 0.75 * yscale;
        // float q = farZ / (farZ - nearZ);
        
        var m = matrix_float4x4()
        m.columns.0 = float4(xscale, 0.0, 0.0, 0.0)
        m.columns.1 = float4(0.0, yscale, 0.0, 0.0)
        m.columns.2 = float4(0.0, 0.0, q, 1.0)
        m.columns.3 = float4(0.0, 0.0, q * -nearZ, 0.0)
    
        return m
    }
    
    func matrix_from_translation(x: Float,
                                 y: Float,
                                 z: Float) -> matrix_float4x4 {
    
        var m = matrix_identity_float4x4
        m.columns.3 = float4(x, y, z, 1.0)
    
        return m
    }
    
    func matrix_from_rotation(radians: Float,
                              x: Float,
                              y: Float,
                              z: Float) -> matrix_float4x4 {
    
        var v = vector_float3(x, y, z)
        v = normalize(v)
        let cos = cosf(radians)
        let cosp = 1.0 - cos
        let sin = sinf(radians)
    
        var m = matrix_float4x4()
        m.columns.0 = float4(cos + cosp * v.x * v.x,
                             cosp * v.x * v.y + v.z * sin,
                             cosp * v.x * v.z - v.y * sin,
                             0.0)
        m.columns.1 = float4(cosp * v.x * v.y - v.z * sin,
                             cos + cosp * v.y * v.y,
                             cosp * v.y * v.z + v.x * sin,
                             0.0)
        m.columns.2 = float4(cosp * v.x * v.z + v.y * sin,
                             cosp * v.y * v.z - v.x * sin,
                             cos + cosp * v.z * v.z,
                             0.0)
        m.columns.3 = float4(0.0,
                             0.0,
                             0.0,
                             1.0)
        return m
    }
}
