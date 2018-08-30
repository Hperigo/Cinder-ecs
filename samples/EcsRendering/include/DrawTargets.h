//
//  DrawTargets.h
//  EcsRendering
//
//  Created by Henrique on 8/29/18.
//

#ifndef DrawTargets_h
#define DrawTargets_h

#include "cinder/gl/gl.h"

using namespace ci;

// simple FBo draw target, everything in it's mDrawables vector will be drawn into it's target fbo. The Draw target itself needs to be added in the manager default draw system
struct FboDrawTarget : public ecs::DrawTarget{
    
    FboDrawTarget(){
        targetFbo = gl::Fbo::create( 300, 300);
    }
    
    FboDrawTarget( const vec2& iSize, const gl::Fbo::Format& iFormat ){
        targetFbo = gl::Fbo::create(iSize.x, iSize.y, iFormat);
    }
    
    
    void update() override {
        /* we can use the update method to do some type of z-sorting, etc... */
    }
    
    
    void draw() override {
        
        {
            vec2 fboSize = targetFbo->getSize();
            targetFbo->bindFramebuffer();
            
            gl::clear( ci::Color(0.5,0.1,0.3) );
            gl::ScopedMatrices sm;
            gl::ScopedViewport sv( fboSize );
            gl::setMatricesWindow(fboSize);
            
            // set matrices, bind FBO etc...
            for(auto& d : mDrawables){
                d->draw();
            }
            targetFbo->unbindFramebuffer();
        }
        
    }
    
    gl::FboRef getFbo(){ return targetFbo; }
    
protected:
    gl::FboRef targetFbo;
};

// blurs everything that it's on the draw call
class BlurFboDrawTarget : public ecs::DrawTarget {
    
public:
    BlurFboDrawTarget( ivec2 size, float targetScale = 1.0f) {
        
        auto format = gl::Fbo::Format();
        mBlurFbo = gl::Fbo::create( size.x * targetScale , size.y * targetScale, format);
        mTempFbo = gl::Fbo::create( size.x * targetScale , size.y * targetScale, format);
        
        mClearColor = ci::ColorA::black();
        mClearColor.a = 0.0f;
        mTargetScale = targetScale;
        
        
        string vertShader = R"(#version 150
        uniform mat4 ciModelViewProjection;
        in vec4 ciPosition;
        in vec2 ciTexCoord0;
        out vec2 vTexCoord0;
        void main()
        {
            vTexCoord0 = ciTexCoord0;
            gl_Position = ciModelViewProjection * ciPosition;
        })"; // end of vert shader
        
        string fragShader = R"(
#version 150
        
        uniform sampler2D    tex0;
        uniform vec2        sample_offset;
        uniform float        attenuation;
        
        in vec2 vTexCoord0;
        
        out vec4 oColor;
        
        void main()
        {
            vec4 sum = vec4( 0.0, 0.0, 0.0, 0.0 );
            sum += texture( tex0, vTexCoord0 + -10.0 * sample_offset ).rgba * 0.009167927656011385;
            sum += texture( tex0, vTexCoord0 +  -9.0 * sample_offset ).rgba * 0.014053461291849008;
            sum += texture( tex0, vTexCoord0 +  -8.0 * sample_offset ).rgba * 0.020595286319257878;
            sum += texture( tex0, vTexCoord0 +  -7.0 * sample_offset ).rgba * 0.028855245532226279;
            sum += texture( tex0, vTexCoord0 +  -6.0 * sample_offset ).rgba * 0.038650411513543079;
            sum += texture( tex0, vTexCoord0 +  -5.0 * sample_offset ).rgba * 0.049494378859311142;
            sum += texture( tex0, vTexCoord0 +  -4.0 * sample_offset ).rgba * 0.060594058578763078;
            sum += texture( tex0, vTexCoord0 +  -3.0 * sample_offset ).rgba * 0.070921288047096992;
            sum += texture( tex0, vTexCoord0 +  -2.0 * sample_offset ).rgba * 0.079358891804948081;
            sum += texture( tex0, vTexCoord0 +  -1.0 * sample_offset ).rgba * 0.084895951965930902;
            sum += texture( tex0, vTexCoord0 +   0.0 * sample_offset ).rgba * 0.086826196862124602;
            sum += texture( tex0, vTexCoord0 +  +1.0 * sample_offset ).rgba * 0.084895951965930902;
            sum += texture( tex0, vTexCoord0 +  +2.0 * sample_offset ).rgba * 0.079358891804948081;
            sum += texture( tex0, vTexCoord0 +  +3.0 * sample_offset ).rgba * 0.070921288047096992;
            sum += texture( tex0, vTexCoord0 +  +4.0 * sample_offset ).rgba * 0.060594058578763078;
            sum += texture( tex0, vTexCoord0 +  +5.0 * sample_offset ).rgba * 0.049494378859311142;
            sum += texture( tex0, vTexCoord0 +  +6.0 * sample_offset ).rgba * 0.038650411513543079;
            sum += texture( tex0, vTexCoord0 +  +7.0 * sample_offset ).rgba * 0.028855245532226279;
            sum += texture( tex0, vTexCoord0 +  +8.0 * sample_offset ).rgba * 0.020595286319257878;
            sum += texture( tex0, vTexCoord0 +  +9.0 * sample_offset ).rgba * 0.014053461291849008;
            sum += texture( tex0, vTexCoord0 + +10.0 * sample_offset ).rgba * 0.009167927656011385;
            
            oColor.rgba = attenuation * sum;
        })";
        
        mBlurShader = gl::GlslProg::create( vertShader, fragShader );
    }
    
    
    void update() override {
        
        
    }
    
    
    void draw() override {
        
        
        {  // draw first pass and scene to the main fbo
            vec2 fboSize = mBlurFbo->getSize();
            mBlurFbo->bindFramebuffer();
            
            gl::clear( mClearColor );
            gl::ScopedMatrices sm;
            gl::ScopedViewport sv( fboSize );
            gl::setMatricesWindow(fboSize);
            
            gl::scale( vec3(  mTargetScale  ) );
            
            // set matrices, bind FBO etc...
            for(auto& d : mDrawables){
                d->draw();
            }
            mBlurFbo->unbindFramebuffer();
        }
        
        

        { // do first blur pass  ( horizontal ... )
            
            gl::ScopedGlslProg sShader( mBlurShader );
            
            mBlurShader->uniform("tex0", 0);
            mBlurShader->uniform("sample_offset",  vec2( blurAmt / mBlurFbo->getWidth(), 0.0f  )  );
            mBlurShader->uniform("attenuation", attenuation );
            
            
            vec2 fboSize = mTempFbo->getSize();
            mTempFbo->bindFramebuffer();
            
            gl::clear( mClearColor );
            gl::ScopedMatrices sm;
            gl::ScopedViewport sv( fboSize );
            gl::setMatricesWindow(fboSize);
            gl::ScopedTextureBind st( mBlurFbo->getColorTexture() );
            
            
            gl::drawSolidRect( Rectf( 0,0,fboSize.x, fboSize.y ) );
            
            mTempFbo->unbindFramebuffer();
            
        }
        
        { // do last pass ( vertical ... )
            
            gl::ScopedGlslProg sShader( mBlurShader );
            
            mBlurShader->uniform("tex0", 0);
            mBlurShader->uniform("sample_offset",  vec2( 0.0f,  blurAmt /  mBlurFbo->getHeight() )  );
            mBlurShader->uniform("attenuation", attenuation );
            
            vec2 fboSize = mBlurFbo->getSize();
            mBlurFbo->bindFramebuffer();
            
            gl::clear( mClearColor );
            gl::ScopedMatrices sm;
            gl::ScopedViewport sv( fboSize );
            gl::setMatricesWindow(fboSize);
            gl::ScopedTextureBind st( mTempFbo->getColorTexture() );
            
            gl::drawSolidRect( Rectf( 0,0,fboSize.x, fboSize.y ) );
            
            mBlurFbo->unbindFramebuffer();
        }

    }
    
    
    gl::FboRef getBluredFbo() const {
        return mBlurFbo;
    }
    
    vec2 getInputSize() const {
        return vec2(mBlurFbo->getSize()) / mTargetScale;
    }
    
    void setClearColor( const ci::ColorA& iColor  ) { mClearColor = iColor; }
    
    float blurAmt = 1.0f;
    float attenuation = 1.0f;
    
protected:
    float mTargetScale = 1.0f;
                      
    ci::ColorA mClearColor;
    
    gl::FboRef mBlurFbo;
    gl::FboRef mTempFbo;
    
    
    gl::GlslProgRef mBlurShader;
};



#endif /* DrawTargets_h */
