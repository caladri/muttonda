#ifndef	PROGRAM_H
#define	PROGRAM_H

class Program {
	std::map<Ref<Name>, Ref<Expression> > definitions_;

public:
	Program(void)
	: definitions_()
	{ }

	~Program()
	{ }

	void begin(bool);

	void define(const Ref<Name>&, const Ref<Expression>&);

	bool defined(const Ref<Name>&);

	void defun(const Function&);

	Ref<Expression> eval(const Ref<Expression>&, bool) const;

	bool load(const std::wstring&);

	void help(bool) const;

	static Program instance_;
};

#endif /* !PROGRAM_H */
