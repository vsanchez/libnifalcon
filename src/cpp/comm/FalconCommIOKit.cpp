#include "falcon/comm/FalconCommIOKit.h"

namespace libnifalcon
{
	void WriteCompletion(void *refCon, IOReturn result, void *arg0)
	{
		printf("Wrote!\n");
		//((FalconCommIOKit*)arg0)->setHasBytesAvailable(true);
		CFRunLoopStop(CFRunLoopGetCurrent());
/*
		IOUSBInterfaceInterface **interface = (IOUSBInterfaceInterface **) refCon;
		UInt32                  numBytesWritten = (UInt32) arg0;
		UInt32                  numBytesRead;

		printf("Asynchronous write complete\n");
		if (result != kIOReturnSuccess)
		{
			printf("error from asynchronous bulk write (%08x)\n", result);
			(void) (*interface)->USBInterfaceClose(interface);
			(void) (*interface)->Release(interface);
			return;
		}

		printf("Wrote (%ld bytes) to bulk endpoint\n", numBytesWritten);

 		numBytesRead = sizeof(gBuffer) - 1; //leave one byte at the end for
                                            //NULL termination

		result = (*interface)->ReadPipeAsync(interface, 9, gBuffer,
											 numBytesRead, ReadCompletion, refCon);

		if (result != kIOReturnSuccess)
		{
			printf("Unable to perform asynchronous bulk read (%08x)\n", result);
			(void) (*interface)->USBInterfaceClose(interface);
			(void) (*interface)->Release(interface);
			return;
		}
*/
	}

	void ReadCompletion(void *refCon, IOReturn result, void *arg0)
	{
		printf("Read!\n");
		//((FalconCommIOKit*)arg0)->setHasBytesAvailable(true);
		CFRunLoopStop(CFRunLoopGetCurrent());
		/*
		IOUSBInterfaceInterface **interface = (IOUSBInterfaceInterface **) refCon;
		UInt32      numBytesRead = (UInt32) arg0;
		UInt32      i;
		printf("Asynchronous bulk read complete\n");
		if (result != kIOReturnSuccess) {
			printf("error from async bulk read (%08x)\n", result);
			(void) (*interface)->USBInterfaceClose(interface);
			(void) (*interface)->Release(interface);
			return;
		}

		printf("Read (%ld bytes) from bulk endpoint\n", numBytesRead);
		*/
	}


	FalconCommIOKit::FalconCommIOKit() :
		m_writeAllocated(false),
		m_readAllocated(false)
	{
		//m_requiresPoll = false;
		if(!m_requiresPoll)
		{
			m_hasBytesAvailable = true;
		}
		IOMasterPort(MACH_PORT_NULL, &m_masterPort);
	}
	
	FalconCommIOKit::~FalconCommIOKit()
	{
		close();
		mach_port_deallocate(mach_task_self(), m_masterPort);
	}
	
	bool FalconCommIOKit::getDeviceCount(int8_t& count)
	{
		kern_return_t			err;
		CFMutableDictionaryRef 	matchingDictionary = 0;		// requires <IOKit/IOKitLib.h>
		SInt32					idVendor = FALCON_VENDOR_ID;
		SInt32					idProduct = FALCON_PRODUCT_ID;
		CFNumberRef				numberRef;
		io_iterator_t			iterator = 0;
		io_service_t			usbDeviceRef;

		count = 0;
		matchingDictionary = IOServiceMatching(kIOUSBDeviceClassName);	// requires <IOKit/usb/IOUSBLib.h>
		if (!matchingDictionary)
		{
			printf("USBSimpleExample: could not create matching dictionary\n");
			return false;
		}
		numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &idVendor);
		if (!numberRef)
		{
			printf("USBSimpleExample: could not create CFNumberRef for vendor\n");
			return false;
		}
		CFDictionaryAddValue(matchingDictionary, CFSTR(kUSBVendorID), numberRef);
		CFRelease(numberRef);
		numberRef = 0;
		numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &idProduct);
		if (!numberRef)
		{
			printf("USBSimpleExample: could not create CFNumberRef for product\n");
			return false;
		}
		CFDictionaryAddValue(matchingDictionary, CFSTR(kUSBProductID), numberRef);
		CFRelease(numberRef);
		numberRef = 0;
		
		err = IOServiceGetMatchingServices(m_masterPort, matchingDictionary, &iterator);
		matchingDictionary = 0;			// this was consumed by the above call
		
		while ( (usbDeviceRef = IOIteratorNext(iterator)) )
		{
			printf("Found device %p\n", (void*)usbDeviceRef);
			++count;
			IOObjectRelease(usbDeviceRef);			// no longer need this reference
		}
		
		IOObjectRelease(iterator);
		iterator = 0;
		
		return true;
	}
	
	bool FalconCommIOKit::open(uint8_t index)
	{
		kern_return_t			err;
		CFMutableDictionaryRef 	matchingDictionary = 0;		// requires <IOKit/IOKitLib.h>
		SInt32					idVendor = FALCON_VENDOR_ID;
		SInt32					idProduct = FALCON_PRODUCT_ID;
		CFNumberRef				numberRef;
		io_iterator_t			iterator = 0;
		io_service_t			usbDeviceRef;

		matchingDictionary = IOServiceMatching(kIOUSBDeviceClassName);	// requires <IOKit/usb/IOUSBLib.h>
		if (!matchingDictionary)
		{
			printf("USBSimpleExample: could not create matching dictionary\n");
			return false;
		}
		numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &idVendor);
		if (!numberRef)
		{
			printf("USBSimpleExample: could not create CFNumberRef for vendor\n");
			return false;
		}
		CFDictionaryAddValue(matchingDictionary, CFSTR(kUSBVendorID), numberRef);
		CFRelease(numberRef);
		numberRef = 0;
		numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &idProduct);
		if (!numberRef)
		{
			printf("USBSimpleExample: could not create CFNumberRef for product\n");
			return false;
		}
		CFDictionaryAddValue(matchingDictionary, CFSTR(kUSBProductID), numberRef);
		CFRelease(numberRef);
		numberRef = 0;
		
		err = IOServiceGetMatchingServices(m_masterPort, matchingDictionary, &iterator);
		matchingDictionary = 0;			// this was consumed by the above call

		int count = 0;
		while ( (usbDeviceRef = IOIteratorNext(iterator)) )
		{
			printf("Found device %p\n", (void*)usbDeviceRef);
			if(count == index) break;
			++count;
			IOObjectRelease(usbDeviceRef);			// no longer need this reference
		}

		IOCFPlugInInterface				**iodev;		// requires <IOKit/IOCFPlugIn.h>
		SInt32							score;
		UInt8							numConf;
		IOUSBConfigurationDescriptorPtr	confDesc;
		IOUSBFindInterfaceRequest		interfaceRequest;
		io_service_t					usbInterfaceRef;
		
		err = IOCreatePlugInInterfaceForService(usbDeviceRef, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &iodev, &score);
		if (err || !iodev)
		{
			printf("dealWithDevice: unable to create plugin. ret = %08x, iodev = %p\n", err, iodev);
			return false;
		}
		err = (*iodev)->QueryInterface(iodev, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID*)&m_falconDevice);
		IODestroyPlugInInterface(iodev);				// done with this
		
		if (err || !m_falconDevice)
		{
			printf("dealWithDevice: unable to create a device interface. ret = %08x, dev = %p\n", err, m_falconDevice);
			return false;
		}
		err = (*m_falconDevice)->USBDeviceOpen(m_falconDevice);
		if (err)
		{
			printf("dealWithDevice: unable to open device. ret = %08x\n", err);
			return false;
		}
		err = (*m_falconDevice)->GetNumberOfConfigurations(m_falconDevice, &numConf);
		if (err || !numConf)
		{
			printf("dealWithDevice: unable to obtain the number of configurations. ret = %08x\n", err);
			close();
			return false;
		}
		printf("dealWithDevice: found %d configurations\n", numConf);
		err = (*m_falconDevice)->GetConfigurationDescriptorPtr(m_falconDevice, 0, &confDesc);			// get the first config desc (index 0)
		if (err)
		{
			printf("dealWithDevice:unable to get config descriptor for index 0\n");
			close();
			return false;
		}
		err = (*m_falconDevice)->SetConfiguration(m_falconDevice, confDesc->bConfigurationValue);
		if (err)
		{
			printf("dealWithDevice: unable to set the configuration\n");
			close();
			return false;
		}

		interfaceRequest.bInterfaceClass = kIOUSBFindInterfaceDontCare;		// requested class
		interfaceRequest.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;		// requested subclass
		interfaceRequest.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;		// requested protocol
		interfaceRequest.bAlternateSetting = kIOUSBFindInterfaceDontCare;		// requested alt setting
    
		err = (*m_falconDevice)->CreateInterfaceIterator(m_falconDevice, &interfaceRequest, &iterator);
		if (err)
		{
			printf("dealWithDevice: unable to create interface iterator\n");
			close();
			return false;
		}

		UInt8					numPipes;

		while ( (usbInterfaceRef = IOIteratorNext(iterator)) )
		{
			err = IOCreatePlugInInterfaceForService(usbInterfaceRef, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID, &iodev, &score);
			if (err || !iodev)
			{
				printf("dealWithInterface: unable to create plugin. ret = %08x, iodev = %p\n", err, iodev);
				return false;
			}
			err = (*iodev)->QueryInterface(iodev, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID), (LPVOID*)&m_falconInterface);
			IODestroyPlugInInterface(iodev);				// done with this
			
			if (err || !m_falconInterface)
			{
				printf("dealWithInterface: unable to create a device interface. ret = %08x, intf = %p\n", err, m_falconInterface);
				return false;
			}
			err = (*m_falconInterface)->USBInterfaceOpen(m_falconInterface);
			if (err)
			{
				printf("dealWithInterface: unable to open interface. ret = %08x\n", err);
				return false;
			}
			err = (*m_falconInterface)->GetNumEndpoints(m_falconInterface, &numPipes);
			if (err)
			{
				printf("dealWithInterface: unable to get number of endpoints. ret = %08x\n", err);
				return false;
			}
			IOObjectRelease(usbInterfaceRef);				// no longer need this reference
		}

		IOReturn kr = (*m_falconInterface)->CreateInterfaceAsyncEventSource(m_falconInterface, &m_runLoopSource);
        if (kr != kIOReturnSuccess)
        {
            printf("Unable to create asynchronous event (%08x)\n", kr);
			close();
			return false;
        }
        CFRunLoopAddSource(CFRunLoopGetCurrent(), m_runLoopSource, kCFRunLoopDefaultMode);

		printf("Opened!");
		return true;
	}
	
	bool FalconCommIOKit::close()
	{
		printf("CLOSING SOMEHOW?!\n");
		IOReturn err;			

		if(m_falconInterface)
		{
			(*m_falconInterface)->USBInterfaceClose(m_falconInterface);
			(*m_falconInterface)->Release(m_falconInterface);
		}
		if(m_falconDevice)
		{
			err = (*m_falconDevice)->USBDeviceClose(m_falconDevice);
			if (err)
			{
				printf("dealWithDevice: error closing device - %08x\n", err);
				(*m_falconDevice)->Release(m_falconDevice);
				return true;
			}
			err = (*m_falconDevice)->Release(m_falconDevice);
			if (err)
			{
				printf("dealWithDevice: error releasing device - %08x\n", err);
				return true;
			}
		}
		return true;
	}
	
	bool FalconCommIOKit::read(uint8_t* buffer, uint32_t size)
	{
		printf("READING?");
		UInt32 s = 64;
		unsigned char b[64];
        IOReturn kr = (*m_falconInterface)->ReadPipeAsync(m_falconInterface, 1, b, s, ReadCompletion, (void*)this);
        if (kr != kIOReturnSuccess)
        {
            printf("Unable to perform bulk read (%08x)\n", kr);
            return false;
        }
		CFRunLoopRun();
		m_lastBytesRead = s - 2;
		b[s] = 0;
		if((s-2) > 0)
			memcpy(buffer, "<AAAAAAAAAAAAAAA>", 16);
//			memcpy(buffer, b+2, s-2);
        printf("Read (%d bytes) from bulk endpoint\n", s-2);
		//buffer
		return true;
	}
	
	bool FalconCommIOKit::write(uint8_t* buffer, uint32_t size)
	{
		unsigned char b[17] = "<AAAAAAAAAAAAAA>";
		if(!m_falconInterface || !(*m_falconInterface))
		{
			printf("CAN'T WRITE?!\n");
			return false;
		}
		//m_writeAllocated = true;
		IOReturn kr = (int)((*m_falconInterface)->GetPipeStatus(m_falconInterface, 2));
		printf("%ld %ld Pipe status: %d\n", m_falconInterface, (*m_falconInterface), kr);
        //kr = (*m_falconInterface)->WritePipe(m_falconInterface, 2, (void*)"<AAAAAAAAAAAAAAA>", 16);
		kr = (*m_falconInterface)->WritePipeAsync(m_falconInterface, 2, (void*)"<AAAAAAAAAAAAAAA>", 16, WriteCompletion, (void*)this);
        if (kr != kIOReturnSuccess)
        {
            printf("Unable to perform bulk write (%08x)\n", kr);
            return false;
        }
		CFRunLoopRun();
		m_lastBytesWritten = size;
        printf("Wrote (%ld bytes) to bulk endpoint\n", size);
		return true;
	}

	void FalconCommIOKit::setHasBytesAvailable(bool v)
	{
//		m_writeAllocated = false;
//		m_hasBytesAvailable = v;
	}
		 
	
	bool FalconCommIOKit::setFirmwareMode()
	{
		return true;
	}
	
	bool FalconCommIOKit::setNormalMode()
	{
		return true;
	}
	
	bool FalconCommIOKit::init()
	{
		return true;
	}
	
	void FalconCommIOKit::poll()
	{
//		if(m_writeAllocated || m_readAllocated)
//			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
	}
	
	void FalconCommIOKit::reset()
	{
	}

}
