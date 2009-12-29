#ifndef	PROGRAM_H
#define	PROGRAM_H

class Program {
	std::map<std::string, Ref<Expression> > definitions_;

public:
	Program(void);

	~Program();

	void begin(bool);

	void define(const std::string&, const Ref<Expression>&);

	bool defined(const std::string&);

	void defun(const SimpleFunction&);

	Ref<Expression> eval(const Ref<Expression>&, bool) const;

	bool load(const std::string&);

	void help(void) const;

	static Program instance_;
};

#endif /* !PROGRAM_H */
