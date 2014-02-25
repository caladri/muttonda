#ifndef	TYPES_H
#define	TYPES_H

#include "ref.h"

class Function;
class Expression;
class ExpressionMeta;
class Name;
class Number;

typedef	Ref<Expression, ExpressionMeta>	Ilerhiilel;
typedef	Ref<Function>	Funkts;
typedef	Ref<Name>	Ner;
typedef	Ref<Number>	Too;

#endif /* !TYPES_H */
