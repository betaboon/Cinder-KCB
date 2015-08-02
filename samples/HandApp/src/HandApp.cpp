/*
* 
* Copyright (c) 2013, Wieden+Kennedy
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "HandTracker.h"
#include "Kinect.h"

class HandApp : public ci::app::App
{
public:
	void 										draw();	
	void										keyDown( ci::app::KeyEvent event );
	void 										setup();
private:
	MsKinect::DeviceRef							mDevice;
	MsKinect::Frame								mFrame;

	std::vector<MsKinect::Hand>					mHands;
	MsKinect::HandTrackerRef					mHandTracker;
};

using namespace ci;
using namespace ci::app;
using namespace std;

void HandApp::draw()
{
	gl::viewport( getWindowSize() );
	gl::clear();
	gl::setMatricesWindow( getWindowSize() );
	
	if ( mFrame.getDepthChannel() ) {

		gl::TextureRef tex = gl::Texture::create( *MsKinect::depthChannelToSurface( mFrame.getDepthChannel() ) );

		gl::color( ColorAf::white() );
		gl::enable( GL_TEXTURE_2D );
		gl::draw( tex, tex->getBounds(), getWindowBounds() );
		gl::disable( GL_TEXTURE_2D );

		gl::pushMatrices();
		gl::scale( vec2( getWindowSize() ) / vec2( tex->getSize() ) );

		for ( const auto& skeleton : mFrame.getSkeletons() ) {
			for ( const auto& joint : skeleton ) {
				const MsKinect::Bone& bone = joint.second;
				ivec2 v0 = mDevice->mapSkeletonCoordToDepth( bone.getPosition() );
				ivec2 v1 = mDevice->mapSkeletonCoordToDepth( skeleton.at( bone.getStartJoint() ).getPosition() );
				gl::drawLine( v0, v1 );
				gl::drawSolidCircle( v0, 5.0f, 16 );
			}
		}

		for ( const auto& hand : mHands ) {
			console() << "drawing hand" << endl;
			for ( const auto& finger : hand.getFingerTipPositions() ) {
				gl::drawStrokedCircle( finger, 5.0f, 24 );
			}
		}

		gl::popMatrices();
	}
}

void HandApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() ) {
	case KeyEvent::KEY_f:
		setFullScreen( !isFullScreen() );
		break;
	case KeyEvent::KEY_q:
		quit();
		break;
	}
}

void HandApp::setup()
{
	setFrameRate( 60.0f );
	glLineWidth( 2.0f );

	mDevice = MsKinect::Device::create();
	mDevice->connectEventHandler( [ & ]( MsKinect::Frame frame )
	{
		mFrame = frame;
		if ( mHandTracker ) {
			mHandTracker->update( mFrame.getDepthChannel(), mFrame.getSkeletons() );
		}
	} );
	try {
		mDevice->start();
	} catch ( MsKinect::Device::ExcDeviceCreate ex ) {
		console() << ex.what() << endl;
	} catch ( MsKinect::Device::ExcDeviceInit ex ) {
		console() << ex.what() << endl;
	} catch ( MsKinect::Device::ExcDeviceInvalid ex ) {
		console() << ex.what() << endl;
	} catch ( MsKinect::Device::ExcGetCoordinateMapper ex ) {
		console() << ex.what() << endl;
	} catch ( MsKinect::Device::ExcOpenStreamColor ex ) {
		console() << ex.what() << endl;
	} catch ( MsKinect::Device::ExcOpenStreamDepth ex ) {
		console() << ex.what() << endl;
	} catch ( MsKinect::Device::ExcStreamStart ex ) {
		console() << ex.what() << endl;
	} catch ( MsKinect::Device::ExcUserTrackingEnable ex ) {
		console() << ex.what() << endl;
	}

	mHandTracker = MsKinect::HandTracker::create();
	mHandTracker->connectEventHandler( [ & ]( vector<MsKinect::Hand> hands )
	{
		console() << "putting hands" << endl;
		mHands = hands;
	} );
	mHandTracker->start( mDevice->getDeviceOptions() );
}

CINDER_APP( HandApp, RendererGl )
