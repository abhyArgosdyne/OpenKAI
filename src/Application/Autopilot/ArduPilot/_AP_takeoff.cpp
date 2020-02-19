#include "../ArduPilot/_AP_takeoff.h"

namespace kai
{

_AP_takeoff::_AP_takeoff()
{
	m_pAP = NULL;
	m_iWP = -1;
}

_AP_takeoff::~_AP_takeoff()
{
}

bool _AP_takeoff::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("iWP",&m_iWP);

	string iName;
	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

bool _AP_takeoff::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_takeoff::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMavlink, -1);

	return this->_AutopilotBase::check();
}

void _AP_takeoff::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateMission();

		this->autoFPSto();
	}
}

void _AP_takeoff::updateMission(void)
{
	IF_(check()<0);

	int apMode = m_pAP->getApMode();
	int apMissionSeq = m_pAP->m_pMavlink->m_mavMsg.m_mission_current.seq;

	if(apMode == ALT_HOLD)
	{
		m_pMC->transit("STANDBY");
		return;
	}

	if(apMode == AUTO && apMissionSeq == m_iWP)
	{
		m_pAP->setApMode(GUIDED);
		return;
	}

	string mission = m_pMC->getCurrentMissionName();

	if(apMode == GUIDED)
	{
		IF_(mission == "RELEASE");

		if(mission != "LAND")
		{
			m_pMC->transit("LAND");
			return;
		}
	}

	if(mission == "RTH")
	{
		m_pAP->setApMode(RTL);
		return;
	}
}

void _AP_takeoff::draw(void)
{
	this->_AutopilotBase::draw();

}

}
