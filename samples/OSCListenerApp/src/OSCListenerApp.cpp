#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "OscListener.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OSCReceiverApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;

	osc::Listener 	listener;
};

void OSCReceiverApp::setup()
{
	listener.setup(3000);
}


void OSCReceiverApp::update()
{

	while (listener.hasWaitingMessages()) {
		osc::Message message;
		listener.getNextMessage(&message);

		console() << "New message received" << std::endl;
		console() << "Address: " << message.getAddress() << std::endl;
		console() << "Num Arg: " << message.getNumArgs() << std::endl;
		for (int i = 0; i < message.getNumArgs(); i++) {
			console() << "-- Argument " << i << std::endl;
			console() << "---- type: " << message.getArgTypeName(i) << std::endl;
			if (message.getArgType(i) == osc::TYPE_INT32) {
				try {
					console() << "------ value: " << message.getArgAsInt32(i) << std::endl;
				}
				catch (...) {
					console() << "Exception reading argument as int32" << std::endl;
				}
			}
			else if (message.getArgType(i) == osc::TYPE_FLOAT) {
				try {
					console() << "------ value: " << message.getArgAsFloat(i) << std::endl;
				}
				catch (...) {
					console() << "Exception reading argument as float" << std::endl;
				}
			}
			else if (message.getArgType(i) == osc::TYPE_STRING) {
				try {
					console() << "------ value: " << message.getArgAsString(i).c_str() << std::endl;
				}
				catch (...) {
					console() << "Exception reading argument as string" << std::endl;
				}
			}
		}
/*
		if (message.getNumArgs() != 0 && message.getArgType(0) == osc::TYPE_FLOAT)
			positionX = message.getArgAsFloat(0);*/
	}
}

void OSCReceiverApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( OSCReceiverApp, RendererGl )
