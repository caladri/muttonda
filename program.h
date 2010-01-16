#ifndef	PROGRAM_H
#define	PROGRAM_H

class Program {
	std::map<std::wstring, Ref<Expression> > definitions_;

public:
	Program(void)
	: definitions_()
	{ }

	~Program()
	{ }

	void begin(bool);

	void define(const std::wstring&, const Ref<Expression>&);

	bool defined(const std::wstring&);

	void defun(const Function&);

	Ref<Expression> eval(const Ref<Expression>&, bool) const;

	bool load(const std::wstring&);

	void help(void) const;

	static Program instance_;
};

#endif /* !PROGRAM_H */
