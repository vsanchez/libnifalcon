/***
 * @file FalconCommIOKit.h
 * @brief Pure IOKit API implementation of FTDI calls because apparently I hate myself.
 * @author Kyle Machulis (kyle@nonpolynomial.com)
 * @version $Id$
 * @copyright (c) 2007-2008 Nonpolynomial Labs/Kyle Machulis
 * @license BSD License
 *
 * $HeadURL$
 * 
 * Project info at http://libnifalcon.sourceforge.net/ 
 *
 */
#ifndef FALCONCOMMIOKIT_H
#define FALCONCOMMIOKIT_H

#include "falcon/core/FalconComm.h"
// Headers for general IOKit definitions
//#include <IOKit/IOLib.h>
//#include <IOKit/IOService.h>
//#include <IOKit/IOMemoryDescriptor.h>
//#include <IOKit/IOMessage.h>

// Headers for USB specific definitions
#include <mach/mach.h>

#include <CoreFoundation/CFNumber.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#include <CoreFoundation/CoreFoundation.h>

namespace libnifalcon
{
	class FalconCommIOKit : public FalconComm
	{
	public:
		FalconCommIOKit();
		~FalconCommIOKit();
		virtual bool getDeviceCount(int8_t& );
		virtual bool open(uint8_t );
		virtual bool close();
		virtual bool read(uint8_t*, uint32_t);
		virtual bool write(uint8_t*, uint32_t);
		virtual bool setFirmwareMode();
		virtual bool setNormalMode();
		void setHasBytesAvailable(bool v);
		bool init();
		void poll();
		void reset();
	protected:
		bool m_writeAllocated;
		bool m_readAllocated;
		IOUSBDeviceInterface	**m_falconDevice;
		IOUSBInterfaceInterface **m_falconInterface;
		CFRunLoopSourceRef m_runLoopSource;
		mach_port_t 	m_masterPort;
		
	};
};

#endif
