from Expression import Division
from Expression import Multiplication
from Expression import Subtraction
from Expression import Variable

def test_multiplication():

    expression = Multiplication(Variable("00"), Variable("01"))

    values = expression.get_possible_values_complete()

    assert(len(values) == 2)
    assert(-1 in values)
    assert(1 in values)

def test_subtraction():
    expression = Subtraction(Variable("00"), Variable("01"))

    values = expression.get_possible_values_complete()
    
    assert(len(values) == 3)
    assert(-2 in values)
    assert(2 in values)
    assert(0 in values)

def test_division():
    expression = Division(Subtraction(Variable("00"), Variable("01")), Subtraction(Variable("10"), Variable("11")))

    values = expression.get_possible_values_complete()
    
    assert(len(values) == 3)
    assert(-1 in values)
    assert(1 in values)
    assert(0 in values)

def test_simplification():
    expression = Subtraction(Multiplication(Variable("10"), Variable("01")), Multiplication(Variable("01"), Variable("10")))

    expression = expression.simplify()

    values = expression.get_possible_values_complete()
    
    assert(len(values) == 1)
    assert(0 in values)

    assert(expression.is_variable)


test_multiplication()
test_subtraction()
test_division()
test_simplification()
