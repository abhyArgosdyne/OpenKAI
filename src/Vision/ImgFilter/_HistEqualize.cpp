/*
 * _HistEqualize.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_HistEqualize.h"

namespace kai
{

	_HistEqualize::_HistEqualize()
	{
		m_type = vision_histEqualize;
		m_pV = NULL;
	}

	_HistEqualize::~_HistEqualize()
	{
		close();
	}

	bool _HistEqualize::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + ": not found");

		return true;
	}

	bool _HistEqualize::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _HistEqualize::close(void)
	{
		this->_VisionBase::close();
	}

	bool _HistEqualize::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _HistEqualize::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpen)
				open();

			m_pT->autoFPSfrom();

			if (m_bOpen)
				filter();

			m_pT->autoFPSto();
		}
	}

	void _HistEqualize::filter(void)
	{
		IF_(m_pV->BGR()->bEmpty());

		Mat mIn;
		Mat mOut;
		vector<Mat> vChannels;

		//Using reference code from:
		//https://opencv-srf.blogspot.jp/2013/08/histogram-equalization.html

		cv::cvtColor(*m_pV->BGR()->m(), mIn, COLOR_BGR2YCrCb); //change the color image from BGR to YCrCb format
		split(mIn, vChannels);								   //split the image into channels
		cv::equalizeHist(vChannels[0], vChannels[0]);		   //equalize histogram on the 1st channel (Y)
		merge(vChannels, mIn);								   //merge 3 channels including the modified 1st channel into one image
		cv::cvtColor(mIn, mOut, COLOR_YCrCb2BGR);			   //change the color image from YCrCb to BGR format (to display image properly)

		m_fBGR.copy(mOut);
	}

}
