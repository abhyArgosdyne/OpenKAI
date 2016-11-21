/*
 * State.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_AUTOMATON_STATE_H_
#define OPENKAI_SRC_AUTOMATON_STATE_H_

#include "../Base/common.h"
#include "../Automaton/Transition.h"
#include "../Script/Kiss.h"

#define N_TRANSITION 32

namespace kai
{

class State
{
public:
	State();
	virtual ~State();

	bool init(Kiss* pKiss);
	bool link(Kiss* pKiss);

	Transition* addTransition(void);
	bool IsValid(void);
	State* Transit(void);

	bool setPtrByName(string name, int* ptr);
	bool setPtrByName(string name, double* ptr);


public:
	string		m_name;
	int			m_nTransition;
	Transition*	m_pTransition[N_TRANSITION];

};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON_STATE_H_ */
