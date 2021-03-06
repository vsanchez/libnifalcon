/***
 * @file FalconCLIBase.cpp
 * @brief Utility class for common operations (device opening, firmware loading, etc...) needed in command line interface examples
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


#include <iostream>

#include <boost/shared_ptr.hpp>

#include "falcon/util/FalconCLIBase.h"
#ifdef LIBFTD2XX
#include "falcon/comm/FalconCommFTD2XX.h"
#endif
#ifdef LIBFTDI
#include "falcon/comm/FalconCommLibFTDI.h"
#endif
#ifdef LIBUSB
#include "falcon/comm/FalconCommLibUSB.h"
#endif
#include "falcon/firmware/FalconFirmwareNovintSDK.h"
#include "falcon/util/FalconFirmwareBinaryTest.h"
#include "falcon/util/FalconFirmwareBinaryNvent.h"


#ifdef ENABLE_LOGGING
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
static const log4cxx::LogString TTCC_CONVERSION_PATTERN(LOG4CXX_STR("%-5p [%c] - %m%n"));

/**
 * Statically initialize the log4cxx library.
 */
void configureLogging(const std::string logString, const log4cxx::LevelPtr level) {
  log4cxx::LayoutPtr layout(new log4cxx::PatternLayout(logString));
  log4cxx::AppenderPtr appender(new log4cxx::ConsoleAppender(layout));
  log4cxx::BasicConfigurator::configure(appender);
  log4cxx::LoggerPtr rootlogger = log4cxx::Logger::getRootLogger();
  rootlogger->setLevel(level);
}
#endif

namespace libnifalcon
{
	namespace po = boost::program_options;

	FalconCLIBase::FalconCLIBase() :
		m_falconDevice(new FalconDevice()),
		m_displayCalibrationMessage(true)
	{
		po::options_description program("Program Options");
		program.add_options()
			("help", "show this help message");
		m_progOptions.add(program);
	}

	void FalconCLIBase::addOptions(int value)
	{
		if(value & COMM_OPTIONS)
		{
			po::options_description comm("Communication Options");
			comm.add_options()
#if defined(LIBUSB)
				("libusb", "use libusb-1.0 based driver")
#endif
#if defined(LIBFTDI)
				("libftdi", "use libftdi based driver")
#elif defined(LIBFTD2XX)
				("ftd2xx", "use ftd2xx based driver")
#endif
				;
			m_progOptions.add(comm);
		}

		if(value & DEVICE_OPTIONS)
		{
			po::options_description device("Device options");
			device.add_options()
				("device_count", "Print the number of devices currently connected and return")
				("device_index", po::value<int>(), "Opens device of given index (starts at 0)")
				;

			m_progOptions.add(device);
		}

		if(value & FIRMWARE_OPTIONS)
		{
			po::options_description firmware("Firmware Options");
			firmware.add_options()
				("nvent_firmware", "Use 'nVent' firmware (Recommended)")
				("test_firmware", "Use test firmware")
				("firmware_file", po::value<std::string>(), "Specify external firmware file (instead of nvent or test)")
				("force_firmware", "Force firmware download, even if already loaded")
				("skip_checksum", "Ignore checksum errors when loading firmware (useful for FTD2XX on non-windows platforms)")
				;
			m_progOptions.add(firmware);
		}

#ifdef ENABLE_LOGGING
		po::options_description debug("Debug Message Options");
		debug.add_options()
			("debug_level", po::value<std::string>(), "Level of debug messages to print (FATAL, ERROR, WARN, INFO, DEBUG) (Default: FATAL)")
			;
//		("output_file", po::value<std::string>(), "File to output debug messages to (outputs to stdout otherwise")
		m_progOptions.add(debug);		
#endif
	}

	void FalconCLIBase::outputProgramOptions()
	{
		std::cout << "Usage: falcon_test_cli [args]" << std::endl;
		std::cout << m_progOptions << std::endl;
	}
	
	bool FalconCLIBase::parseOptions(int argc, char** argv)
	{
		try
		{
			po::store(po::parse_command_line(argc, argv, m_progOptions), m_varMap);
			po::notify(m_varMap);
		}
		catch(...)
		{
			std::cout << "Invalid Arguments. Please try again." << std::endl << std::endl;
			outputProgramOptions();
			return false;
		}
		
		if (m_varMap.count("help")) {
			outputProgramOptions();
			return false;
		}

#ifdef ENABLE_LOGGING
		std::string logPattern(TTCC_CONVERSION_PATTERN);
		log4cxx::LevelPtr logLevel = log4cxx::Level::toLevel("FATAL");

		if(m_varMap.count("debug_level"))
		{
			logLevel = log4cxx::Level::toLevel(m_varMap["debug_level"].as<std::string>());
		}

		configureLogging(logPattern, logLevel);
#endif
		
		m_falconDevice->setFalconFirmware<FalconFirmwareNovintSDK>();
		
		//First off, see if we have a communication method
		if(m_varMap.count("libftdi") && m_varMap.count("ftd2xx"))
		{
			std::cout << "Error: can only use one comm method. Choose either libftdi or ftd2xx, depending on which is available." << std::endl;
			return false;
		}
		
		//This is an either/or choice, since we have problems with static linking and ftd2xx. Prefer libusb1, then libftdi. Thanks for the static linking against old libusb binaries, FTDI!

#if defined(LIBUSB)
		else if (m_varMap.count("libusb"))
		{
			std::cout << "Setting up libusb device" << std::endl;
			m_falconDevice->setFalconComm<FalconCommLibUSB>();
		}
#endif
#if defined(LIBFTDI)
		else if (m_varMap.count("libftdi"))
		{
			std::cout << "Setting up libftdi device" << std::endl;
			m_falconDevice->setFalconComm<FalconCommLibFTDI>();
		}
#elif defined(LIBFTD2XX)
		else if (m_varMap.count("ftd2xx"))
		{
			std::cout << "Setting up ftd2xx device" << std::endl;
			m_falconDevice->setFalconComm<FalconCommFTD2XX>();
		}
#endif
        else
		{
            std::cout << "No communication method selected." << std::endl;
			outputProgramOptions();
            return false;
        }


		//Device count check
		if(m_varMap.count("device_count"))
		{
			int8_t count;
			m_falconDevice->getDeviceCount(count);
			std::cout << "Connected Device Count: " << (int)count << std::endl;
			return false;
		}		
		else if(m_varMap.count("device_index"))
		{
			if(!m_falconDevice->open(m_varMap["device_index"].as<int>()))
			{
				std::cout << "Cannot open falcon device index " << m_varMap["device_index"].as<int>() << " - Lib Error Code: " << m_falconDevice->getErrorCode() << " Device Error Code: " << m_falconDevice->getFalconComm()->getDeviceErrorCode() << std::endl;
				return false;
			}
		}
		else
		{
			std::cout << "No device index specified to open, cannot continue (--help for options)" << std::endl;
			return false;			
		}

		//There's only one kind of firmware right now, so automatically set that.
		m_falconDevice->setFalconFirmware<FalconFirmwareNovintSDK>();
		//See if we have firmware
		bool firmware_loaded = m_falconDevice->isFirmwareLoaded();	 
		if((m_varMap.count("firmware") || m_varMap.count("test_firmware") || m_varMap.count("nvent_firmware")) && (m_varMap.count("force_firmware") || !firmware_loaded))
		{
			std::cout << "Loading firmware" << std::endl;
			if(m_varMap.count("nvent_firmware"))
			{
				for(int i = 0; i < 10; ++i)
				{
					if(!m_falconDevice->getFalconFirmware()->loadFirmware(m_varMap.count("skip_checksum") > 0, NOVINT_FALCON_NVENT_FIRMWARE_SIZE, const_cast<uint8_t*>(NOVINT_FALCON_NVENT_FIRMWARE)))
					{
						std::cout << "Could not load firmware" << std::endl;
					}
					else
					{
						std::cout <<"Firmware loaded" << std::endl;
						break;
					}
				}
			}
			else if(m_varMap.count("test_firmware"))
			{
				for(int i = 0; i < 10; ++i)
				{
					if(!m_falconDevice->getFalconFirmware()->loadFirmware(m_varMap.count("skip_checksum") > 0, NOVINT_FALCON_TEST_FIRMWARE_SIZE, const_cast<uint8_t*>(NOVINT_FALCON_TEST_FIRMWARE)))
					{
						std::cout << "Could not load firmware" << std::endl;
					}
					else
					{
						std::cout <<"Firmware loaded" << std::endl;
						break;
					}
				}
			}
			else if(m_varMap.count("firmware"))
			{
				//Check for existence of firmware file
				std::string firmware_file = m_varMap["firmware"].as<std::string>();
				if(!m_falconDevice->setFirmwareFile(firmware_file))
				{
					std::cout << "Cannot find firmware file - " << firmware_file << std::endl;
					return false;
				}
				if(!m_falconDevice->loadFirmware(10, m_varMap.count("skip_checksum") > 0))
				{
					std::cout << "Cannot load firmware to device" << std::endl;
					std::cout << "Error Code: " << m_falconDevice->getErrorCode() << std::endl;
					if(m_falconDevice->getErrorCode() == 2000)
					{
						std::cout << "Device Error Code: " << m_falconDevice->getFalconComm()->getDeviceErrorCode() << std::endl;
					}
					return false;
				}
			}
		}
		else if(!firmware_loaded)
		{
			std::cout << "No firmware loaded to device, and no firmware specified to load (--nvent_firmware, --test_firmware, etc...). Cannot continue" << std::endl;
			return false;
		}
		else
		{
			return true;
		}
		if(!m_falconDevice->isFirmwareLoaded())
		{
			std::cout << "No firmware loaded to device, cannot continue" << std::endl;
			return false;
		}
		
		return true;
	}

	bool FalconCLIBase::calibrateDevice()
	{
		bool homing = false;
		m_falconDevice->getFalconFirmware()->setHomingMode(true);
		m_falconDevice->runIOLoop();
		if(!m_falconDevice->getFalconFirmware()->isHomed())
		{
			if(!homing)
			{
				m_falconDevice->getFalconFirmware()->setLEDStatus(libnifalcon::FalconFirmware::RED_LED);
				if(m_displayCalibrationMessage)
				{
					std::cout << "Falcon not currently calibrated. Move control all the way out then push straight all the way in." << std::endl;
					m_displayCalibrationMessage = false;
				}
			}
			homing = true;
			return false;
		}
		std::cout << "Falcon calibrated successfully." << std::endl;
		m_falconDevice->getFalconFirmware()->setLEDStatus(libnifalcon::FalconFirmware::GREEN_LED);
		homing = false;
		return true;
	}
}
