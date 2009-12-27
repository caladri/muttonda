#ifndef	PROGRAM_H
#define	PROGRAM_H

class Program {
	std::map<std::string, Ref<Expression> > definitions_;

public:
	Program(void);

	~Program();

	void begin(bool) const;

	void define(const std::string&, const Ref<Expression>&);

	bool defined(const std::string&);

	void defun(const std::string&, const std::vector<Name>&, const Ref<Expression>&);

	void defun(const std::string&, const Ref<Expression>&);

	void defun(const SimpleFunction&);

	Ref<Expression> eval(const Ref<Expression>&, bool) const;

	void help(void) const;

	static Program instance_;
};

#endif /* !PROGRAM_H */
