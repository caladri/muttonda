#include <math.h>

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "expression.h"
#include "function.h"
#include "lambda.h"
#include "name.h"

#include "church.h"

Expression
apply(const Expression& a, const Expression& b)
{
	return (Expression(a, b));
}

Expression
apply(const Expression& a, const Expression& b, const Expression& c)
{
	return (Expression(Expression(a, b), c));
}

Expression
lambda(const Name& v, const Expression& e)
{
	return (Expression(Lambda(v, e)));
}

int
main(void)
{
	try {
		std::cout << Expression(Expression(Add, Scalar(1)), Scalar(2)) << std::endl;
		std::cout << Expression(Expression(Add, Scalar(1)), Scalar(2)).eval() << std::endl;

		Expression tree(Expression(Expression(cons, Expression(Expression(cons, Scalar(1)), Scalar(2))),
					   Expression(Expression(cons, Scalar(3)), Scalar(4))));

		std::cout << tree << std::endl;

		std::cout << Expression(car, Expression(car, tree)).eval() << std::endl;
		std::cout << Expression(cdr, Expression(car, tree)).eval() << std::endl;
		std::cout << Expression(car, Expression(cdr, tree)).eval() << std::endl;
		std::cout << Expression(cdr, Expression(cdr, tree)).eval() << std::endl;

		std::cout << Expression(cond) << std::endl;
		std::cout << Expression(cond).eval() << std::endl;

		std::cout << apply(cond, False, Scalar(2)) << std::endl;
		std::cout << apply(cond, False, Scalar(2)).eval() << std::endl;

		std::cout << apply(cond, True, Scalar(2)) << std::endl;
		std::cout << apply(cond, True, Scalar(2)).eval() << std::endl;

		std::cout << apply(apply(cond, False, Scalar(2)), False) << std::endl;
		std::cout << apply(apply(cond, False, Scalar(2)), False).eval() << std::endl;

		std::cout << apply(apply(cond, True, Scalar(2)), False) << std::endl;
		std::cout << apply(apply(cond, True, Scalar(2)), False).eval() << std::endl;

		std::cout << apply(Church, Scalar(2)) << std::endl;
		std::cout << apply(Church, Scalar(2)).eval() << std::endl;

		std::cout << apply(Church, Scalar(0)) << std::endl;
		std::cout << apply(Church, Scalar(0)).eval() << std::endl;

		std::cout << apply(Church, Scalar(10)) << std::endl;
		std::cout << apply(Church, Scalar(10)).eval() << std::endl;

		std::cout << Expression(unchurch) << std::endl;
		std::cout << Expression(unchurch).eval() << std::endl;

		std::cout << apply(unchurch, apply(Church, Scalar(5))) << std::endl;
		std::cout << apply(unchurch, apply(Church, Scalar(5))).eval() << std::endl;

		std::cout << apply(unchurch, apply(plus, apply(Church, Scalar(7)), apply(Church, Scalar(5)))) << std::endl;
		std::cout << apply(unchurch, apply(plus, apply(Church, Scalar(7)), apply(Church, Scalar(5)))).eval() << std::endl;

		std::cout << apply(unchurch, apply(mult, apply(Church, Scalar(7)), apply(Church, Scalar(5)))) << std::endl;
		std::cout << apply(unchurch, apply(mult, apply(Church, Scalar(7)), apply(Church, Scalar(5)))).eval() << std::endl;

		std::cout << apply(unchurch, apply(expn, apply(Church, Scalar(7)), apply(Church, Scalar(5)))) << std::endl;
		std::cout << apply(unchurch, apply(expn, apply(Church, Scalar(7)), apply(Church, Scalar(5)))).eval() << std::endl;

		return (0);
	} catch (const char *msg) {
		std::cerr << "Exception: " << msg << std::endl;

		return (1);
	}
}
