/*
 * Window.h
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_Windows_H_
#define OpenKAI_src_UI_Windows_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "../Vision/Frame.h"

namespace kai
{

#define TAB_PIX 20
#define LINE_HEIGHT 20
#define MOUSE_L 1
#define MOUSE_M (1<<1)
#define MOUSE_R (1<<2)

class Window: public BASE
{
public:
	Window();
	virtual ~Window();

	bool init(void* pKiss);
	bool draw(void);
	Frame* getFrame(void);
	Point* getTextPos(void);

	void addMsg(const string& pMsg);
	void tabNext(void);
	void tabPrev(void);
	void tabReset(void);
	void lineNext(void);
	void lineReset(void);
	double textSize(void);
	Scalar textColor(void);

	bool bMouseButton(uint32_t fB);

	static void OnMouse(int event, int x, int y, int flags, void* pW)
	{
		Window* pWin = (Window*) pW;
		Frame* pF = pWin->getFrame();
		NULL_(pF);
		IF_(pF->bEmpty());

		switch (event)
		{
		case EVENT_MOUSEMOVE:
			pWin->m_vMouse.x = (float)x/(float)pF->m()->cols;
			pWin->m_vMouse.y = (float)y/(float)pF->m()->rows;
			break;
		case EVENT_LBUTTONDOWN:
			pWin->m_fMouse |= MOUSE_L;
			break;
		case EVENT_LBUTTONUP:
			pWin->m_fMouse &= ~MOUSE_L;
			break;
		case EVENT_RBUTTONDOWN:
			pWin->m_fMouse |= MOUSE_R;
			break;
		case EVENT_RBUTTONUP:
			pWin->m_fMouse &= ~MOUSE_R;
			break;
		case EVENT_MBUTTONDOWN:
			pWin->m_fMouse |= MOUSE_M;
			break;
		case EVENT_MBUTTONUP:
			pWin->m_fMouse &= ~MOUSE_M;
			break;
		default:
			break;
		}
	}

public:
	bool m_bWindow;
	bool m_bFullScreen;
	Frame m_frame;
	vInt2 m_size;
	vInt2 m_textPos;
	vInt2 m_textStart;
	int m_pixTab;
	int m_lineHeight;
	Point m_tPoint;
	double m_textSize;
	Scalar m_textCol;

	string m_gstOutput;
	VideoWriter m_gst;

	string m_fileRec;
	VideoWriter m_VW;

	Frame m_F;
	Frame m_F2;

	//mouse handler
	bool	m_bMouse;
	vFloat2 m_vMouse;
	uint32_t m_fMouse;
	bool	m_bShowMouse;


};

}

#endif
