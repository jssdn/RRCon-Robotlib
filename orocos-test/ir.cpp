#include "ir.hpp"

#include <native/task.h>

ir::ir(std::string name, std::string propertyfilename):
        TaskContext(name)
{
       
}

ir::~ir()
{
}

bool ir::startHook()
{
	log(RTT::Info) << this->getName() << " startHook() was called!" << RTT::endlog();
	if (rt_task_self() == NULL){
		log(RTT::Error) << this->getName() << "ir::startHook(): Not a Xenomai realtime task!" << RTT::endlog();
		return false;
	}
	int err;
	if( (err = spi_init(&spi, "/dev/spi0", 0, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0)) < 0 ){
		log(RTT::Error) << this->getName() << "ir::startHook(): Unable to initialize SPI. Task will NOT start!" << RTT::endlog();
		return false;
	}
	if( (err = max1231_init(&this->adc, &spi)) < 0 ){
		log(RTT::Error) << this->getName() << "ir::startHook(): Unable to initialize ADC. Task will NOT start!" << RTT::endlog();
		return false;
	}
	max1231_config(&this->adc);
	return true;
}


void ir::updateHook()
{
	log(RTT::Info) << this->getName() << " updateHook() was called!" << RTT::endlog();
	// FIXME: Is it smart to usleep() in the middle of a periodic task???
	adc_reset(&this->adc); 
	__usleep(10000); 
	adc_config_all_uni_single(&this->adc);
	__usleep(10000);

	int ret;
	adc_get_temperature(&adc, &ret);
	log(RTT::Info) << this->getName() << " ADC_TASK: Temp: " << (float)ret/8 << RTT::endlog();
	adc_reset_fifo(&this->adc);
	return;
}

void ir::stopHook()
{
	log(RTT::Info) << this->getName() << " stopHook() was called!" << RTT::endlog();
	return;
}

bool ir::breakUpdateHook()
{
	return true;
}
