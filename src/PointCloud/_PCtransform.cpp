/*
 * _PCtransform.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCtransform.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D

namespace kai
{

_PCtransform::_PCtransform()
{
	m_vT.init(0);
	m_vR.init(0);
}

_PCtransform::~_PCtransform()
{
}

bool _PCtransform::init(void *pKiss)
{
	IF_F(!_PointCloudBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vT", &m_vT);
	pK->v("vR", &m_vR);

	int nM;
	pK->v("nMt", &nM);
	for(; nM > 0; nM--)
		m_vmT.push_back(Eigen::Matrix4d::Identity());

	return true;
}

bool _PCtransform::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _PCtransform::check(void)
{
	NULL__(m_pPCB, -1);

	return 0;
}

void _PCtransform::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateTransform();
		m_sPC.update();

		this->autoFPSto();
	}
}

void _PCtransform::updateTransform(void)
{
	IF_(check()<0);

	Eigen::Matrix4d mT = Eigen::Matrix4d::Identity();
	Eigen::Vector3d vR(m_vR.x, m_vR.y, m_vR.z);
	mT.block(0,0,3,3) = m_sPC.next()->GetRotationMatrixFromXYZ(vR);
	mT(0,3) = m_vT.x;
	mT(1,3) = m_vT.y;
	mT(2,3) = m_vT.z;

	for(Eigen::Matrix4d m : m_vmT)
		mT *= m;

	m_sPC.next()->Transform(mT);
}

void _PCtransform::setTranslation(vFloat3& vT)
{
	m_vT = vT;
}

void _PCtransform::setRotation(vFloat3& vR)
{
	m_vR = vR;
}

void _PCtransform::setTranslationMatrix(int i, Eigen::Matrix4d_u& mR)
{
	IF_(i >= m_vmT.size());
	m_vmT[i] = mR;
}

}
#endif
#endif
