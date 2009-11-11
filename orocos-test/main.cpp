#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Activity.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>
#include <rtt/Event.hpp>
#include <rtt/Logger.hpp>
#include <rtt/os/main.h>
#include <rtt/PeriodicActivity.hpp>
#include <ocl/TaskBrowser.hpp>

#include <iostream>

#include "ir.hpp"

using namespace Orocos;

int ORO_main (int argc, char **argv)
{
	RTT::TaskContext *irblob = NULL;

	

	//set log level to more verbose than default
        if (log().getLogLevel() < Logger::Debug) {
                log().setLogLevel(Logger::Debug);
                log(Info) << argv[0] << " manually raises LogLevel to 'Debug'. " << endlog();
        }
	
	irblob = new ir("IRSensor");
	irblob->getActivity()->thread()->setPeriod(1);
	irblob->getActivity()->thread()->setScheduler(ORO_SCHED_RT);
	irblob->getActivity()->thread()->setPriority(RTT::OS::HighestPriority);

	TaskBrowser browser (irblob);
	connectPeers(irblob, &browser);

	browser.loop();

	irblob->stop();
	delete irblob;

	return (0);

}
