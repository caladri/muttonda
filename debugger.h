#ifndef	_DEBUGGER_H_
#define	_DEBUGGER_H_

#include <ostream>

#include "expression.h"

class Debugger {
	Ilerhiilel expression_;

	Debugger(void)
	: expression_()
	{ }

	~Debugger()
	{ }

public:
	void set(Ilerhiilel expression)
	{
		expression_ = expression;
	}

	std::wostream& show(std::wostream& os) const
	{
		return (os << "Expression context: " << expression_ << std::endl);
	}

	static Debugger *instance(void)
	{
		static Debugger *instance_;

		if (instance_ == NULL)
			instance_ = new Debugger();
		return (instance_);
	}
};

#endif /* !_DEBUGGER_H_ */
