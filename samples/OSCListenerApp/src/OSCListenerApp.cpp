#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "OscListener.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Bone {
public:
	ivec2 v0;
	ivec2 v1;

	Bone(ivec2 _v0, ivec2 _v1) { v0 = _v0; v1 = _v1; }
};
typedef std::vector<Bone> Skeleton;

class OSCListenerApp : public App {
public:
	#define MESSAGE_START 0xDEADBABE
	#define MESSAGE_END   0xDEADBEEF
	#define MESSAGE_FRAME_SIZE 6
	#define MESSAGE_BONE_SIZE 5
	void setup() override;
	void update() override;
	void draw() override;

	osc::Listener 	mListener;
	std::map<int, Skeleton> mSkeletons;

};

void OSCListenerApp::setup()
{
	mListener.setup(3000);
	mSkeletons.clear();
}

void OSCListenerApp::update()
{

	while (mListener.hasWaitingMessages()) {
		osc::Message message;
		mListener.getNextMessage(&message);
		//console() << "Address: " << message.getAddress() << endl;
		if (message.getNumArgs() < MESSAGE_FRAME_SIZE)  {
			console() << "Message too small!" << endl;
			continue;
		}
		try {
			if (
				MESSAGE_START != message.getArgAsInt32( 0 ) ||
				MESSAGE_END != message.getArgAsInt32( message.getNumArgs() - 1 )
				) {
				console() << "Invalid message!" << endl;
				continue;
			}

			ivec2 scale = ivec2( message.getArgAsInt32( 1 ), message.getArgAsInt32( 2 ) );
			int skeletonSize = message.getArgAsInt32( 3 );
			int skeletonId = message.getArgAsInt32( 4 );

			if ((message.getNumArgs() - MESSAGE_FRAME_SIZE) != skeletonSize * MESSAGE_BONE_SIZE) {
				console() << "Bones are missing!" << endl;
				continue;
			}

			mSkeletons.erase( skeletonId );

			gl::pushMatrices();
			gl::scale( vec2( getWindowSize() ) / vec2( scale ) );

			Skeleton skeleton;

			for (int i = 0; i < skeletonSize; i++) {
				int offset = MESSAGE_FRAME_SIZE + i * MESSAGE_BONE_SIZE;
				int boneId = message.getArgAsInt32( offset );
				ivec2 v0 = ivec2(
					message.getArgAsInt32( offset + 1 ),
					message.getArgAsInt32( offset + 2 )
					);
				ivec2 v1 = ivec2(
					message.getArgAsInt32( offset + 3 ),
					message.getArgAsInt32( offset + 4 )
					);
				skeleton.push_back( Bone( v0, v1 ) );
				//console() << skeletonId << " - bone -- " << boneId << "/" << skeletonSize << " --- " << v0 << " - " << v1 << endl;
			}
			mSkeletons[skeletonId] = skeleton;

			gl::popMatrices();
		}
		catch (...) {
			console() << "Exception reading an argument" << endl;
		}
	}
}

void OSCListenerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	for (std::map<int, Skeleton>::iterator it = mSkeletons.begin(); it != mSkeletons.end(); ++it) {
		for (Bone bone : it->second) {
			gl::drawLine( bone.v0, bone.v1 );
			gl::drawSolidCircle( bone.v0, 5.0f, 16 );
		}
	}
}

CINDER_APP( OSCListenerApp, RendererGl )
