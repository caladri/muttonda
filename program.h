#ifndef	PROGRAM_H
#define	PROGRAM_H

class Program {
	std::map<std::string, Expression> definitions_;

public:
	Program(void);

	~Program();

	void begin(void) const;

	void define(const std::string&, const Expression&);

	void defun(const std::string&, const std::vector<Name>&, const Expression&);

	void defun(const std::string&, const Expression&, const char * = NULL, ...);

	Expression eval(const Expression&, bool) const;
};

#endif /* !PROGRAM_H */
