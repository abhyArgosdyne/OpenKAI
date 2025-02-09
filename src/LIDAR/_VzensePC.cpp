/*
 * _VzensePC.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_VzensePC.h"

namespace kai
{

	_VzensePC::_VzensePC()
	{
		m_nDevice = 0;
		m_deviceURI = "";
		m_pDeviceListInfo = NULL;
		m_deviceHandle = 0;

		m_bOpen = false;
		m_vSize.set(1600, 1200);
		m_tWait = 80;

		m_btRGB = false;
		m_btDepth = false;
		m_bIR = false;

		m_vzfRGB = {0};
		m_vzfDepth = {0};
		m_vzfIR = {0};

		//		m_psmTransformedDepth = NULL;

		m_pVzVw = NULL;
		m_camCtrl.m_vRz.set(0.0, FLT_MAX);
	}

	_VzensePC::~_VzensePC()
	{
	}

	bool _VzensePC::init(void *pKiss)
	{
		IF_F(!_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("URI", &m_deviceURI);
		pK->v("vSize", &m_vSize);
		pK->v("tWait", &m_tWait);

		pK->v("btRGB", &m_btRGB);
		pK->v("btDepth", &m_btDepth);
		pK->v("bIR", &m_bIR);

		m_camCtrl.m_vRz = {m_vRange.x, m_vRange.y};

		return true;
	}

	bool _VzensePC::open(void)
	{
		IF_T(m_bOpen);

		uint32_t m_nDevice = 0;
		VzReturnStatus status = VZ_GetDeviceCount(&m_nDevice);
		if (status != VzReturnStatus::VzRetOK)
		{
			LOG_E("VzGetDeviceCount failed");
			return false;
		}

		LOG_I("Get device count: " + i2str(m_nDevice));
		IF_F(m_nDevice == 0);

		//        VZ_SetHotPlugStatusCallback(HotPlugStateCallback, nullptr);

		m_pDeviceListInfo = new VzDeviceInfo[m_nDevice];
		status = VZ_GetDeviceInfoList(m_nDevice, m_pDeviceListInfo);
		if (status != VzReturnStatus::VzRetOK)
		{
			LOG_E("GetDeviceListInfo failed:" + i2str(status));
			return false;
		}

		m_deviceHandle = 0;
		if (m_deviceURI.empty())
			m_deviceURI = string(m_pDeviceListInfo[0].uri);
		status = VZ_OpenDeviceByUri(m_deviceURI.c_str(), &m_deviceHandle);
		if (status != VzReturnStatus::VzRetOK)
		{
			LOG_E("OpenDevice failed");
			return false;
		}

		status = VZ_GetSensorIntrinsicParameters(m_deviceHandle, VzToFSensor, &m_cameraParameters);
		cout << "Get VZ_GetSensorIntrinsicParameters status: " << status << endl;
		cout << "ToF Sensor Intinsic: " << endl;
		cout << "Fx: " << m_cameraParameters.fx << endl;
		cout << "Cx: " << m_cameraParameters.cx << endl;
		cout << "Fy: " << m_cameraParameters.fy << endl;
		cout << "Cy: " << m_cameraParameters.cy << endl;
		cout << "ToF Sensor Distortion Coefficient: " << endl;
		cout << "K1: " << m_cameraParameters.k1 << endl;
		cout << "K2: " << m_cameraParameters.k2 << endl;
		cout << "P1: " << m_cameraParameters.p1 << endl;
		cout << "P2: " << m_cameraParameters.p2 << endl;
		cout << "K3: " << m_cameraParameters.k3 << endl;
		cout << "K4: " << m_cameraParameters.k4 << endl;
		cout << "K5: " << m_cameraParameters.k5 << endl;
		cout << "K6: " << m_cameraParameters.k6 << endl;

		const int nBfw = 64;
		char fw[nBfw] = {0};
		VZ_GetFirmwareVersion(m_deviceHandle, fw, nBfw);
		LOG_I("fw  ==  " + string(fw));
		LOG_I("sn  ==  " + string(m_pDeviceListInfo[0].serialNumber));

		VZ_SetFrameRate(m_deviceHandle, (int)this->m_pT->getTargetFPS());
		VZ_SetColorResolution(m_deviceHandle, m_vSize.x, m_vSize.y);
		VZ_SetColorPixelFormat(m_deviceHandle, VzPixelFormatBGR888);
		VZ_SetTransformColorImgToDepthSensorEnabled(m_deviceHandle, m_btRGB);
		VZ_SetTransformDepthImgToColorSensorEnabled(m_deviceHandle, m_btDepth);
		setToFexposureTime(m_camCtrl.m_bAutoExposureToF, m_camCtrl.m_tExposureToF);
		setToFexposureControlMode(m_camCtrl.m_bAutoExposureToF);
		setRGBexposureTime(m_camCtrl.m_bAutoExposureRGB, m_camCtrl.m_tExposureToF);
		setRGBexposureControlMode(m_camCtrl.m_bAutoExposureRGB);
		setTimeFilter(m_camCtrl.m_bFilTime, m_camCtrl.m_filTime);
		setConfidenceFilter(m_camCtrl.m_bFilConfidence, m_camCtrl.m_filConfidence);
		setFlyingPixelFilter(m_camCtrl.m_bFilFlyingPix, m_camCtrl.m_filFlyingPix);
		setFillHole(m_camCtrl.m_bFillHole);
		setSpatialFilter(m_camCtrl.m_bSpatialFilter);
		setHDR(m_camCtrl.m_bHDR);
		status = VZ_StartStream(m_deviceHandle);

		m_pVzVw = new VzVector3f[m_vSize.x * m_vSize.y];

		//		m_tWait = 2 * 1000 / this->m_pT->getTargetFPS();
		m_bOpen = true;
		return true;
	}

	void _VzensePC::close(void)
	{
		VzReturnStatus status = VZ_StopStream(m_deviceHandle);
		status = VZ_CloseDevice(&m_deviceHandle);
		m_deviceHandle = 0;
		LOG_I("CloseDevice status: " + i2str(status));

		status = VZ_Shutdown();

		DEL(m_pDeviceListInfo);
		DEL(m_pVzVw);
	}

	bool _VzensePC::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _VzensePC::check(void)
	{
		NULL__(m_pT, -1);
		return this->_PCframe::check();
	}

	void _VzensePC::update(void)
	{
		VzReturnStatus status = VZ_Initialize();
		if (status != VzReturnStatus::VzRetOK)
		{
			LOG_E("VzInitialize failed");
			return;
		}

		while (m_pT->bRun())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					LOG_E("Cannot open Vzense");
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPSfrom();

			if (updateVzense())
			{
				updatePC();
			}

			m_pT->autoFPSto();
		}
	}

	bool _VzensePC::updateVzense(void)
	{
		IF_T(check() < 0);

		VzFrameReady frameReady = {0};
		VzReturnStatus status = VZ_GetFrameReady(m_deviceHandle,
												 m_tWait,
												 &frameReady);
		Eigen::Affine3d mA = m_A;
		IF_F(status != VzReturnStatus::VzRetOK);
		IF_F(frameReady.transformedColor != 1);
		IF_F(frameReady.depth != 1);

		status = VZ_GetFrame(m_deviceHandle, VzDepthFrame, &m_vzfDepth);
		IF_F(status != VzReturnStatus::VzRetOK);
		IF_F(!m_vzfDepth.pFrameData);

		status = VZ_GetFrame(m_deviceHandle, VzTransformColorImgToDepthSensorFrame, &m_vzfRGB);
		IF_F(status != VzReturnStatus::VzRetOK);
		IF_F(!m_vzfRGB.pFrameData);

		PointCloud *pPC = m_sPC.next();
		const static float s_b = 1.0 / 1000.0;
		const static float c_b = 1.0 / 255.0;

		// Convert Depth frame to World vectors.
		VZ_ConvertDepthFrameToPointCloudVector(m_deviceHandle,
											   &m_vzfDepth,
											   m_pVzVw);

		int nPi = 0;
		for (int i = 0; i < m_vzfDepth.height; i++)
		{
			for (int j = 0; j < m_vzfDepth.width; j++)
			{
				int k = i * m_vzfDepth.width + j;

				VzVector3f *pV = &m_pVzVw[k];
				Eigen::Vector3d vP(pV->x, pV->y, pV->z);
				vP *= s_b;
				IF_CONT(vP.z() < m_camCtrl.m_vRz.x);
				IF_CONT(vP.z() > m_camCtrl.m_vRz.y);

				Eigen::Vector3d vPik = Vector3d(
					vP[m_vAxisIdx.x] * m_vAxisK.x,
					vP[m_vAxisIdx.y] * m_vAxisK.y,
					vP[m_vAxisIdx.z] * m_vAxisK.z);
				vP = mA * vPik;
				pPC->points_.push_back(vP);

				// texture color
				uint8_t *pC = &m_vzfRGB.pFrameData[k * sizeof(uint8_t) * 3];
				Eigen::Vector3d vC(pC[2], pC[1], pC[0]);
				vC *= c_b;
				pPC->colors_.push_back(vC);

				nPi++;
				if (nPi >= m_nPresv)
					break;
			}

			if (nPi >= m_nPresv)
				break;
		}

		return true;
	}

	bool _VzensePC::setToFexposureControlMode(bool bAuto)
	{
		VzReturnStatus vzR = VZ_SetExposureControlMode(m_deviceHandle,
													   VzToFSensor,
													   bAuto ? VzExposureControlMode_Auto
															 : VzExposureControlMode_Manual);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _VzensePC::setToFexposureTime(bool bAuto, int tExposure)
	{
		VzExposureTimeParams p;
		p.mode = bAuto ? VzExposureControlMode_Auto
					   : VzExposureControlMode_Manual;
		p.exposureTime = tExposure;
		VzReturnStatus vzR = VZ_SetExposureTime(m_deviceHandle,
												VzToFSensor,
												p);

		return (vzR == VzRetOK) ? true : false;
	}

	bool _VzensePC::setRGBexposureControlMode(bool bAuto)
	{
		VzReturnStatus vzR = VZ_SetExposureControlMode(m_deviceHandle,
													   VzColorSensor,
													   bAuto ? VzExposureControlMode_Auto
															 : VzExposureControlMode_Manual);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _VzensePC::setRGBexposureTime(bool bAuto, int tExposure)
	{
		VzExposureTimeParams p;
		p.mode = bAuto ? VzExposureControlMode_Auto
					   : VzExposureControlMode_Manual;
		p.exposureTime = tExposure;
		VzReturnStatus vzR = VZ_SetExposureTime(m_deviceHandle,
												VzColorSensor,
												p);

		return (vzR == VzRetOK) ? true : false;
	}

	bool _VzensePC::setTimeFilter(bool bON, int thr)
	{
		VzTimeFilterParams p;
		p.enable = bON;
		p.threshold = thr;
		VzReturnStatus vzR = VZ_SetTimeFilterParams(m_deviceHandle, p);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _VzensePC::setConfidenceFilter(bool bON, int thr)
	{
		VzConfidenceFilterParams p;
		p.enable = bON;
		p.threshold = thr;
		VzReturnStatus vzR = VZ_SetConfidenceFilterParams(m_deviceHandle, p);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _VzensePC::setFlyingPixelFilter(bool bON, int thr)
	{
		VzFlyingPixelFilterParams p;
		p.enable = bON;
		p.threshold;
		VzReturnStatus vzR = VZ_SetFlyingPixelFilterParams(m_deviceHandle, p);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _VzensePC::setFillHole(bool bON)
	{
		VzReturnStatus vzR = VZ_SetFillHoleFilterEnabled(m_deviceHandle, bON);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _VzensePC::setSpatialFilter(bool bON)
	{
		VzReturnStatus vzR = VZ_SetSpatialFilterEnabled(m_deviceHandle, bON);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _VzensePC::setHDR(bool bON)
	{
		VzReturnStatus vzR = VZ_SetHDRModeEnabled(m_deviceHandle, bON);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _VzensePC::setCamCtrl(const VzCamCtrl &camCtrl)
	{
		m_camCtrl.m_vRz = camCtrl.m_vRz;

		if ((m_camCtrl.m_tExposureToF != camCtrl.m_tExposureToF) || (m_camCtrl.m_bAutoExposureToF != camCtrl.m_bAutoExposureToF))
		{
			IF_F(!setToFexposureTime(camCtrl.m_bAutoExposureToF,
									 camCtrl.m_tExposureToF));
			IF_F(!setToFexposureControlMode(camCtrl.m_bAutoExposureToF));

			m_camCtrl.m_bAutoExposureToF = camCtrl.m_bAutoExposureToF;
			m_camCtrl.m_tExposureToF = camCtrl.m_tExposureToF;
		}

		if ((m_camCtrl.m_tExposureRGB != camCtrl.m_tExposureRGB) || (m_camCtrl.m_bAutoExposureRGB != camCtrl.m_bAutoExposureRGB))
		{
			IF_F(!setRGBexposureTime(camCtrl.m_bAutoExposureRGB,
									 camCtrl.m_tExposureRGB));
			IF_F(!setRGBexposureControlMode(camCtrl.m_bAutoExposureRGB));

			m_camCtrl.m_bAutoExposureRGB = camCtrl.m_bAutoExposureRGB;
			m_camCtrl.m_tExposureRGB = camCtrl.m_tExposureRGB;
		}

		if ((m_camCtrl.m_filTime != camCtrl.m_filTime) || (m_camCtrl.m_bFilTime != camCtrl.m_bFilTime))
		{
			IF_F(!setTimeFilter(camCtrl.m_bFilTime,
								camCtrl.m_filTime));

			m_camCtrl.m_bFilTime = camCtrl.m_bFilTime;
			m_camCtrl.m_filTime = camCtrl.m_filTime;
		}

		if ((m_camCtrl.m_filConfidence != camCtrl.m_filConfidence) || (camCtrl.m_bFilConfidence != camCtrl.m_bFilConfidence))
		{
			IF_F(!setConfidenceFilter(camCtrl.m_bFilConfidence,
									  camCtrl.m_filConfidence));

			m_camCtrl.m_bFilConfidence = camCtrl.m_bFilConfidence;
			m_camCtrl.m_filConfidence = camCtrl.m_filConfidence;
		}

		if ((m_camCtrl.m_filFlyingPix != camCtrl.m_filFlyingPix) || (m_camCtrl.m_bFilFlyingPix != camCtrl.m_bFilFlyingPix))
		{
			IF_F(!setFlyingPixelFilter(camCtrl.m_bFilFlyingPix,
									   camCtrl.m_filFlyingPix));

			m_camCtrl.m_bFilFlyingPix = camCtrl.m_bFilFlyingPix;
			m_camCtrl.m_filFlyingPix = camCtrl.m_filFlyingPix;
		}

		if (m_camCtrl.m_bFillHole != camCtrl.m_bFillHole)
		{
			IF_F(!setFillHole(camCtrl.m_bFillHole));

			m_camCtrl.m_bFillHole = camCtrl.m_bFillHole;
		}

		if (m_camCtrl.m_bSpatialFilter != camCtrl.m_bSpatialFilter)
		{
			IF_F(!setSpatialFilter(camCtrl.m_bSpatialFilter));

			m_camCtrl.m_bSpatialFilter = camCtrl.m_bSpatialFilter;
		}

		if (m_camCtrl.m_bHDR != camCtrl.m_bHDR)
		{
			IF_F(!setHDR(camCtrl.m_bHDR));

			m_camCtrl.m_bHDR = camCtrl.m_bHDR;
		}

		return true;
	}

}
