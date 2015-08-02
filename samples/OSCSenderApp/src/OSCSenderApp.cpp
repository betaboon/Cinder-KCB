#include <boost/format.hpp>

#include "cinder/app/App.h"
#include "cinder/System.h"

#include "OscSender.h"
#include "Kinect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OSCSenderApp : public App {
  public:
	void setup() override;
	void setupKinect();
	void setupOSC(int port);
	void draw() override;
private:
	MsKinect::DeviceRef			mDevice;
	MsKinect::Frame				mFrame;

	osc::Sender					mSender;
	std::string					mHost;
	int 						mPort;
};
void OSCSenderApp::setupKinect()
{
	mDevice = MsKinect::Device::create();
	mDevice->connectEventHandler([&](MsKinect::Frame frame)
	{
		mFrame = frame;
	});
	try {
		mDevice->start();
	}
	catch (MsKinect::Device::ExcDeviceCreate ex) {
		console() << ex.what() << endl;
	}
	catch (MsKinect::Device::ExcDeviceInit ex) {
		console() << ex.what() << endl;
	}
	catch (MsKinect::Device::ExcDeviceInvalid ex) {
		console() << ex.what() << endl;
	}
	catch (MsKinect::Device::ExcGetCoordinateMapper ex) {
		console() << ex.what() << endl;
	}
	catch (MsKinect::Device::ExcOpenStreamColor ex) {
		console() << ex.what() << endl;
	}
	catch (MsKinect::Device::ExcOpenStreamDepth ex) {
		console() << ex.what() << endl;
	}
	catch (MsKinect::Device::ExcStreamStart ex) {
		console() << ex.what() << endl;
	}
	catch (MsKinect::Device::ExcUserTrackingEnable ex) {
		console() << ex.what() << endl;
	}
}

void OSCSenderApp::setupOSC(int port = 3000)
{
	mPort = port;
	// assume the broadcast address is this machine's IP address but with 255 as the final value
	// so to multicast from IP 192.168.1.100, the host should be 192.168.1.255
	mHost = System::getIpAddress();
	if (mHost.rfind('.') != string::npos)
		mHost.replace(mHost.rfind('.') + 1, 3, "255");
	mSender.setup(mHost, port, true);
}
void OSCSenderApp::setup()
{
	setFrameRate(60.0f);
	setupOSC();
	setupKinect();
}

void OSCSenderApp::draw()
{
	if (mFrame.getDepthChannel()) {
		int i = 0;
		for (const auto& skeleton : mFrame.getSkeletons()) {
			console() << " -- skeleton - " << ++i << " --" << endl;
			osc::Message message;
			message.setAddress((boost::format("/cinder/osc/kinect/skeleton/%d") % i).str());
			int j = 0;
			for (const auto& joint : skeleton) {
				console() << "bone" << ++j;
				const MsKinect::Bone& bone = joint.second;

				ivec2 v0 = mDevice->mapSkeletonCoordToDepth(bone.getPosition());
				ivec2 v1 = mDevice->mapSkeletonCoordToDepth(skeleton.at(bone.getStartJoint()).getPosition());
				/*gl::drawLine(v0, v1);
				gl::drawSolidCircle(v0, 5.0f, 16);*/
				message.addIntArg(v0.x);
				message.addIntArg(v0.y);
				message.addIntArg(v1.x);
				message.addIntArg(v1.y);
				console() << v0 << " - " << v1 << endl;
			}
			mSender.sendMessage(message);
		}

		//if (mFace.getMesh2d()->getNumVertices() > 0) {
		//	gl::pushMatrices();
		//	gl::scale(0.5f, 0.5f);
		//	gl::color(ColorAf::white());
		//	gl::enableWireframe();
		//	gl::draw(*mFace.getMesh2d());
		//	gl::disableWireframe();
		//	gl::popMatrices();
		//}
		//gl::popMatrices();
	}
}
CINDER_APP(OSCSenderApp, Renderer2d)
