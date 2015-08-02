#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "OscListener.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OSCReceiverApp : public App {
public:
	#define MESSAGE_START 0xDEADBABE
	#define MESSAGE_END   0xDEADBEEF
	#define MESSAGE_FRAME_SIZE 4
	void setup() override;
	void update() override;
	void draw() override;

	osc::Listener 	mListener;
};

void OSCReceiverApp::setup()
{
	mListener.setup(3000);
}


void OSCReceiverApp::update()
{

	while (mListener.hasWaitingMessages()) {
		osc::Message message;
		mListener.getNextMessage(&message);
		console() << "Address: " << message.getAddress() << endl;
		if (message.getNumArgs() < MESSAGE_FRAME_SIZE)  {
			console() << "Message too small!" << endl;
			continue;
		}
		try {
			if (
				MESSAGE_START != message.getArgAsInt32(0) ||
				MESSAGE_END != message.getArgAsInt32(message.getNumArgs() - 1)
				) {
				console() << "Invalid message!" << endl;
				continue;
			}

			int skeletonId = message.getArgAsInt32(1);
			int skeletonSize = message.getArgAsInt32(2);

			if ((message.getNumArgs() - MESSAGE_FRAME_SIZE) != skeletonSize * 5) {
				console() << "Bones are missing!" << endl;
				continue;
			}

			for (int i = 0; i < skeletonSize; i++) {
				int offset = MESSAGE_FRAME_SIZE + i * 5;
				int boneId = message.getArgAsInt32(offset);
				ivec2 v0 = ivec2(
					message.getArgAsInt32(offset + 1),
					message.getArgAsInt32(offset + 2)
					);
				ivec2 v1 = ivec2(
					message.getArgAsInt32(offset + 3),
					message.getArgAsInt32(offset + 4)
					);
				console() << skeletonId << " - bone -- " << boneId << "/" << skeletonSize << " --- " << v0 << " - " << v1 << endl;
			}
		}
		catch (...) {
			console() << "Exception reading an argument" << endl;
		}
	}
}

void OSCReceiverApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( OSCReceiverApp, RendererGl )
