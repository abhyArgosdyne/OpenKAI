/*
 * _SlideWindowDetector.h
 *
 *  Created on: June 2, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__SlideWindowDetector_H_
#define OpenKAI_src_Detector__SlideWindowDetector_H_

#include "../Base/common.h"
#include "_DNNclassifier.h"
#include "../Vision/_DepthVisionBase.h"

namespace kai
{

class _SlideWindowDetector : public _DetectorBase
{
public:
	_SlideWindowDetector();
	virtual ~_SlideWindowDetector();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	bool draw(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SlideWindowDetector*) This)->update();
		return NULL;
	}

public:
	_DNNclassifier* m_pC;
	_VisionBase* m_pD;

	Mat		m_mD;
	Mat		m_mDin;
	Mat		m_mBGR;
	float	m_w;
	float	m_dW;
	int		m_nW;
	float	m_maxD;
	float	m_minArea;
	int		m_nLevel;
	vFloat2	m_dRange;
};
}

#endif
