/***
 * @file FalconFirmware.h
 * @brief Base class for firmware policy classes
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

#ifndef FALCONFIRMWARE_H
#define FALCONFIRMWARE_H

#include <stdint.h>
#include <string>
#include <cstdlib>
#include <deque>
#include "boost/array.hpp"
#include "boost/shared_ptr.hpp"
#include "falcon/core/FalconComm.h"
#include "falcon/core/FalconLogger.h"

namespace libnifalcon
{
	class FalconFirmware : public FalconCore
	{
	public:
		enum
		{
			GREEN_LED=0x2,		/**< Flag to control Green LED */
			BLUE_LED=0x4,		/**< Flag to control Blue LED */
			RED_LED=0x8			/**< Flag to control Red LED  */
		};

		enum
		{
			ENCODER_1_HOMED = 0x1, /**< Flag for encoder homing for motor 1 */
			ENCODER_2_HOMED = 0x2, /**< Flag for encoder homing for motor 2 */
			ENCODER_3_HOMED = 0x4, /**< Flag for encoder homing for motor 3 */
		};

		enum {
			FALCON_FIRMWARE_NO_COMM_SET = 3000, /**< Error for no communications policy set */
			FALCON_FIRMWARE_NO_FIRMWARE_SET, /**< Error for no firmware policy set */
			FALCON_FIRMWARE_NO_FIRMWARE_LOADED, /**< Error for no firmware loaded */
			FALCON_FIRMWARE_FILE_NOT_VALID, /**< Error for firmware file missing */
			FALCON_FIRMWARE_CHECKSUM_MISMATCH /**< Error for checksum mismatch during firmware loading */
		};


		/**
		 * Constructor
		 *
		 *
		 */
		FalconFirmware();

		/**
		 * Destructor
		 *
		 *
		 */
		virtual ~FalconFirmware()
		{
			//Don't do anything with the falcon communications
			//Assume that's managed elsewhere
		}

		/**
		 * Run one read/write loop
		 *
		 *
		 * @return True if read and write are successful, false otherwise
		 */
		virtual bool runIOLoop() = 0;

		/**
		 * Returns the size of grip information for this firmware.
		 *
		 * This may or may not work. However, there's currently only one grip type, so I'm guessing.
		 *
		 * @return Size of the buffer that getGripInfo will return
		 */
		virtual int32_t getGripInfoSize() = 0;

		/**
		 * Retrieves the buffer containing grip data
		 *
		 *
		 * @return Byte buffer with grip data
		 */
		virtual uint8_t* getGripInfo() = 0;

		/**
		 * Sets the instantious force (in whatever units the falcon takes) for the next I/O loop
		 *
		 * @param force Array of signed 16-bit integers to set force
		 */
		void setForces(const boost::array<int16_t, 3> (&force))
		{
			m_forceValues[0] = force[0];
			m_forceValues[1] = force[1];
			m_forceValues[2] = force[2];
		}
		/**
		 * Returns an array of current motor encoder values from the last I/O loop
		 *
		 *
		 * @return Signed 16-bit integers representing the current encoder values for each motor
		 */
		int16_t* getEncoderValues() { return m_encoderValues; }

		/**
		 * Sets the LEDs to turn on or off at the next I/O loop
		 *
		 * @param leds Bitfield of the LED flags
		 */
		void setLEDStatus(uint8_t leds) { m_ledStatus = leds; }

		/**
		 * Returns the current LED status bitfield
		 *
		 *
		 * @return The current LED status bitfield
		 */
		uint8_t getLEDStatus() { return m_ledStatus; }

		/**
		 * Sets the homing mode for the next I/O loop
		 *
		 * @param value True to turn homing mode on, false for off
		 */
		void setHomingMode(bool value) { m_homingMode = value; }

		/**
		 * Return the homing status of each encoder
		 *
		 *
		 * @return Bitfield of encoder homing statuses
		 */
		uint8_t getHomingModeStatus() { return m_homingStatus; }

		/**
		 * Returns the overall homing status
		 *
		 * Basically, if only one encoder is homed, it doesn't do you a lot of good. This is a conveinence function to signify
		 * whether all motors are homed
		 *
		 * @return True if all encoders are homed, false otherwise
		 */
		bool isHomed() { return ( (m_homingStatus & (ENCODER_1_HOMED | ENCODER_2_HOMED | ENCODER_3_HOMED)) > 0); }

		/**
		 * Sets the communication object for the firmware to run I/O through
		 *
		 * @param f Pointer to the communications object
		 */
		void setFalconComm(boost::shared_ptr<FalconComm> f) { m_falconComm = f; }

		/**
		 * Checks to see if firmware is loaded by running IO loop 10 times, returning true on first success
		 * Will automatically return false is setFalconFirmware() has not been called.
		 *
		 * @return true if firmware is loaded, false otherwise
		 */
		bool isFirmwareLoaded();

		/**
		 * Sets the firmware file to load to the falcon
		 *
		 * @param filename Name of the file to use for firmware
		 *
		 * @return true if file exists and is openable, false otherwise
		 */
		bool setFirmwareFile(const std::string& filename);

        /**
		 * Conveinence function, calls loadFirmware with a certain number of retries
		 *
		 * @param retries Number of times to retry loading firmware before quitting
		 * @param skip_checksum Whether or not to skip checksum tests when loading firmware (useful with ftd2xx on non-windows platforms)
		 *
		 * @return true if firmware is loaded successfully, false otherwise
		 */
		bool loadFirmware(int retries, bool skip_checksum = false);

        /**
		 * Conveinence function, runs one try of loading firmware with set filename
		 *
		 * @param skip_checksum Whether or not to skip checksum tests when loading firmware (useful with ftd2xx on non-windows platforms)
		 *
		 * @return true if firmware is loaded successfully, false otherwise
		 */
		bool loadFirmware(bool skip_checksum = false);

		/**
		 * Tries to load the firmware that was specified by the setFirmwareFile function. For the moment, skip_checksum should be used
		 * on non-windows platforms when using the ftd2xx communications core, as there is a bug that causes bad checksum returns on
		 * otherwise proper firmware loading events
		 *
		 * @param skip_checksum Whether or not to skip checksum tests when loading firmware (useful with ftd2xx on non-windows platforms)
		 *
		 * @return true if firmware is loaded successfully, false otherwise
		 */
		bool loadFirmware(bool skip_checksum, const long& firmware_size, uint8_t* buffer);

		/**
		 * Used to reset the state of the communications if reloading firmware more than once in the same session
		 *
		 */
		virtual void resetFirmwareState()
		{
			m_hasWritten = false;
		}

		uint64_t getLoopCount() { return m_loopCount; }
	protected:
		boost::shared_ptr<FalconComm> m_falconComm; /**< Communications object for I/O */
		std::string m_firmwareFilename; /**< Filename of the firmware to load */
		bool m_isFirmwareLoaded; /**< True if firmware has been loaded, false otherwise */

		//Values sent to falcon
		bool m_homingMode;		/**< True if homing mode is on, false for homing mode off */
		uint8_t m_ledStatus;	/**< Bitfield for LED Status */
		int16_t m_forceValues[3]; /**< Force values for the next I/O loop */
		//Values received from falcon
		int16_t m_encoderValues[3];	/**< Encoder values from the last I/O loop */
		uint8_t m_homingStatus; /**< Current homing status from the last I/O loop */

		uint64_t m_loopCount; /**< Number of successful loops that have been run by this firmware instance */
		bool m_hasWritten; /**< True if we're waiting for a read return */
	private:
		DECLARE_LOGGER();

	};
}
#endif
