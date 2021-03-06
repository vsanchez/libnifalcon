/***
 * @file FalconKinematicStamper.cpp
 * @brief IK and DK implementation for the Novint Falcon based on paper by R.E. Stamper (http://libnifalcon.wiki.sourceforge.net/space/showimage/PhD_97-4.pdf)
 * @author Kevin Ouellet (kouellet@users.sourceforge.net) / Kyle Machulis (kyle@nonpolynomial.com)
 * @version $Id$
 * @copyright (c) 2007-2008 Nonpolynomial Labs/Kyle Machulis
 * @license BSD License
 *
 * $HeadURL$
 * 
 * Project info at http://libnifalcon.sourceforge.net/ 
 *
 */

#include "falcon/kinematic/FalconKinematicStamper.h"
#include "falcon/kinematic/stamper/JacobianMatrix.h"

namespace libnifalcon
{
	inline float clamp(float x, float a, float b)
	{
		return x < a ? a : (x > b ? b : x);
	}

	FalconKinematicStamper::FalconKinematicStamper(bool init_now)
	{
		if(init_now)
		{
			initialize();
		}
	}

	void FalconKinematicStamper::initialize()
	{
		m_dir.initialize();
	}

	bool FalconKinematicStamper::getForces(const double (&position)[3], const double (&cart_force)[3], int16_t (&enc_force)[3])
	{
		double ang[3];
		StamperKinematicImpl::Angle a;
		gmtl::Point3f p(position[0], position[1], position[2]);
		gmtl::Vec3f c(cart_force[0], cart_force[1], cart_force[2]);
		//Our gain is 10000, I guess. This is just the number that made the black box match the falcon outputs.
		//I do not question the magic.
		c *= 10000.0;
		a = m_inv.calculate(p);
	
		gmtl::Vec3f angularVelocity = StamperKinematicImpl::JacobianMatrix::calculate(a, c);
		for(int i = 0; i < 3; ++i)
		{
			enc_force[i] = clamp(-angularVelocity[i], -4095, 4095);
		}
		return true;
	}
	
	bool FalconKinematicStamper::getAngles(const double (&position)[3], double (&angles)[3])
	{
		StamperKinematicImpl::Angle a;
		gmtl::Point3f p(position[0], position[1], position[2]);
		a = m_inv.calculate(p);
			
		angles[0] = a.theta1[0];
		angles[1] = a.theta1[1];
		angles[2] = a.theta1[2];			
		return true;
	}

	bool FalconKinematicStamper::getPosition(const int16_t (&encoders)[3], double (&position)[3])
	{
		float angle1 = getTheta(encoders[0]);
		float angle2 = getTheta(encoders[1]);
		float angle3 = getTheta(encoders[2]);
		gmtl::Vec3f v = gmtl::Vec3f(gmtl::Math::deg2Rad(angle1), gmtl::Math::deg2Rad(angle2), gmtl::Math::deg2Rad(angle3));
		gmtl::Point3f p = m_dir.calculate(v);
		position[0] = p[0];
		position[1] = p[1];
		position[2] = p[2];
		return true;
	}
}
