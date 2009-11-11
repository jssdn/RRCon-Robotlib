#ifndef __IR__
#define __IR__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>
#include <rtt/Event.hpp>
#include <rtt/Logger.hpp>

extern "C" {
#include <util.h>
#include <xspidev.h>
#include <max1231adc.h>
}
#include <sys/types.h>

class ir: public RTT::TaskContext{
public: 
        ir(std::string name, std::string propertyfilename="ir.cpf");
        virtual ~ir();

        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook();
        virtual bool breakUpdateHook();
private:
	MAX1231 adc;
	XSPIDEV spi;
	uint8_t dest[32];
};

#endif //__IR__
