/*
 * BASE.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_BASE_BASE_H_
#define OPENKAI_SRC_BASE_BASE_H_

#include "common.h"
#include "../Stream/Frame.h"
#include "Message.h"

using namespace std;

namespace kai
{


class BASE
{
public:
	BASE();
	virtual ~BASE();

	virtual bool init(void* pKiss);
	virtual bool link(void);

	string* getName(void);
	string* getClass(void);

	virtual bool start(void);
	virtual bool draw(Frame* pFrame, vInt4* pTextPos);

public:
	void*	m_pKiss;
};

}

#endif /* OPENKAI_SRC_BASE_BASE_H_ */
