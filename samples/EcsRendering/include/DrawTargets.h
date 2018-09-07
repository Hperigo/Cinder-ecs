//
//  DrawTargets.h
//  EcsRendering
//
//  Created by Henrique on 8/29/18.
//

#ifndef DrawTargets_h
#define DrawTargets_h

#include "cinder/gl/gl.h"


// simple FBo draw target, everything in it's mDrawables vector will be drawn into it's target fbo. The Draw target itself needs to be added in the manager default draw system
struct FboDrawTarget : public ecs::DrawTarget{
    
    FboDrawTarget(){
        targetFbo = ci::gl::Fbo::create( 300, 300);
    }
    
    FboDrawTarget( const glm::vec2& iSize, const ci::gl::Fbo::Format& iFormat ){
        targetFbo = ci::gl::Fbo::create(iSize.x, iSize.y, iFormat);
    }
    
    
    void update() override {
        /* we can use the update method to do some type of z-sorting, etc... */
    }
    
    
    void draw() override {
        
        {
            glm::vec2 fboSize = targetFbo->getSize();
            targetFbo->bindFramebuffer();
            
            ci::gl::clear( ci::Color(0.5,0.1,0.3) );
            ci::gl::ScopedMatrices sm;
            ci::gl::ScopedViewport sv( fboSize );
            ci::gl::setMatricesWindow(fboSize);
            
            // set matrices, bind FBO etc...
            for(auto& d : mDrawables){
                d->draw();
            }
            targetFbo->unbindFramebuffer();
        }
        
    }
    
    ci::gl::FboRef getFbo(){ return targetFbo; }
    
protected:
    ci::gl::FboRef targetFbo;
};

// blurs everything that it's on the draw call
class BlurFboDrawTarget : public ecs::DrawTarget {
    
public:
    BlurFboDrawTarget( glm::ivec2 size, float targetScale = 1.0f) {
        
        auto format = ci::gl::Fbo::Format();
        mBlurFbo = ci::gl::Fbo::create( size.x * targetScale , size.y * targetScale, format);
        mTempFbo = ci::gl::Fbo::create( size.x * targetScale , size.y * targetScale, format);
        
        mClearColor = ci::ColorA::black();
        mClearColor.a = 0.0f;
        mTargetScale = targetScale;
        
        
        std::string vertShader = R"(#version 150
        uniform mat4 ciModelViewProjection;
        in vec4 ciPosition;
        in vec2 ciTexCoord0;
        out vec2 vTexCoord0;
        void main()
        {
            vTexCoord0 = ciTexCoord0;
            gl_Position = ciModelViewProjection * ciPosition;
        })"; // end of vert shader
        
        std::string fragShader = R"(
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
        
        mBlurShader = ci::gl::GlslProg::create( vertShader, fragShader );
    }
    
    
    void update() override {
        
        
    }
    
    
    void draw() override {
        
        
        {  // draw first pass and scene to the main fbo
            glm::vec2 fboSize = mBlurFbo->getSize();
            mBlurFbo->bindFramebuffer();
            
            ci::gl::clear( mClearColor );
            ci::gl::ScopedMatrices sm;
            ci::gl::ScopedViewport sv( fboSize );
            ci::gl::setMatricesWindow(fboSize);
            
            ci::gl::scale( glm::vec3(  mTargetScale  ) );
            
            // set matrices, bind FBO etc...
            for(auto& d : mDrawables){
                d->draw();
            }
            mBlurFbo->unbindFramebuffer();
        }
        
        

        { // do first blur pass  ( horizontal ... )
            
            ci::gl::ScopedGlslProg sShader( mBlurShader );
            
            mBlurShader->uniform("tex0", 0);
            mBlurShader->uniform("sample_offset",  glm::vec2( blurAmt / mBlurFbo->getWidth(), 0.0f  )  );
            mBlurShader->uniform("attenuation", attenuation );
            
            
            glm::vec2 fboSize = mTempFbo->getSize();
            mTempFbo->bindFramebuffer();
            
            ci::gl::clear( mClearColor );
            ci::gl::ScopedMatrices sm;
            ci::gl::ScopedViewport sv( fboSize );
            ci::gl::setMatricesWindow(fboSize);
            ci::gl::ScopedTextureBind st( mBlurFbo->getColorTexture() );
            
            
            ci::gl::drawSolidRect( ci::Rectf( 0,0,fboSize.x, fboSize.y ) );
            
            mTempFbo->unbindFramebuffer();
            
        }
        
        { // do last pass ( vertical ... )
            
            ci::gl::ScopedGlslProg sShader( mBlurShader );
            
            mBlurShader->uniform("tex0", 0);
            mBlurShader->uniform("sample_offset",  glm::vec2( 0.0f,  blurAmt /  mBlurFbo->getHeight() )  );
            mBlurShader->uniform("attenuation", attenuation );
            
            glm::vec2 fboSize = mBlurFbo->getSize();
            mBlurFbo->bindFramebuffer();
            
            ci::gl::clear( mClearColor );
            ci::gl::ScopedMatrices sm;
            ci::gl::ScopedViewport sv( fboSize );
            ci::gl::setMatricesWindow(fboSize);
            ci::gl::ScopedTextureBind st( mTempFbo->getColorTexture() );
            
            ci::gl::drawSolidRect( ci::Rectf( 0,0,fboSize.x, fboSize.y ) );
            
            mBlurFbo->unbindFramebuffer();
        }

    }
    
    
    ci::gl::FboRef getBluredFbo() const {
        return mBlurFbo;
    }
    
    glm::vec2 getInputSize() const {
        return glm::vec2(mBlurFbo->getSize()) / mTargetScale;
    }
    
    void setClearColor( const ci::ColorA& iColor  ) { mClearColor = iColor; }
    
    float blurAmt = 1.0f;
    float attenuation = 1.0f;
    
protected:
    float mTargetScale = 1.0f;
                      
    ci::ColorA mClearColor;
    
    ci::gl::FboRef mBlurFbo;
    ci::gl::FboRef mTempFbo;
    
    
    ci::gl::GlslProgRef mBlurShader;
};



#endif /* DrawTargets_h */
