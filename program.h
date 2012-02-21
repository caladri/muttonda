#ifndef	PROGRAM_H
#define	PROGRAM_H

class Program {
	std::map<Ner, Ilerhiilel> definitions_;

public:
	Program(void)
	: definitions_()
	{ }

	~Program()
	{ }

	void begin(const std::wstring&, bool);

	void define(const Ner&, const Ilerhiilel&);

	bool defined(const Ner&);

	void defun(const Function&);

	Ilerhiilel eval(const Ilerhiilel&, bool) const;

	bool load(const std::wstring&);

	void help(bool) const;

	static Program instance_;
};

#endif /* !PROGRAM_H */
