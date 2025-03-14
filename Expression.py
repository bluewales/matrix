import math
import random

class Expression:
    def __init__(self, left_operand, right_operand):
        self.left_operand: Expression = left_operand
        self.right_operand: Expression = right_operand

        self.is_addition = False
        self.is_division = False
        self.is_multiplication = False
        self.is_subtration = False
        self.is_variable = False
        self.is_condition = False
        self.is_conditional_add = False

        self.simplified = False

        self.get_dependent_veriable_limit = 10
        self.c_variable_length = 30

        self.hash = None
        self.c = None
        self.dependent_veriables = None

        self.bits = None
        self.trailing_zeros = None

        self.all_factors_are_variables = False

    def evaluate(self, values):
        left_result = self.left_operand.evaluate(values)
        right_result = self.right_operand.evaluate(values)

        return self.operate(left_result, right_result)
    

    def simplify(self):

        if self.simplified:
            return
        self.simplified = True

        self.left_operand = self.left_operand.simplify()
        self.right_operand = self.right_operand.simplify()
        
    def try_all_variables(self, settings, variables):
        if len(variables) == 0:
            value = self.evaluate(settings)
            result = set()

            if not math.isnan(value):
                result.add(value)
                
            return result
        variable = variables.pop()
        
        settings[variable] = 1
        possitive_results = self.try_all_variables(settings, variables)
        settings[variable] = -1
        negative_results = self.try_all_variables(settings, variables)

        variables.add(variable)
        return possitive_results.union(negative_results)

    def get_possible_values_complete(self):

        return self.try_all_variables({}, self.get_dependent_veriables())

    def __str__(self):
        return self.get_string()

    def get_string(self):
        return "(" + self.left_operand.get_string() + self.operation + self.right_operand.get_string() + ")"
    
    def get_polish(self):
        if self.hash is None:
            left_polish = self.left_operand.get_polish()
            right_polish = self.right_operand.get_polish()
            if len(left_polish) > self.c_variable_length:
                left_polish = hex(abs(hash(left_polish)))[2:]
            if len(right_polish) > self.c_variable_length:
                right_polish = hex(abs(hash(right_polish)))[2:]
            self.hash = self.polish_letter + left_polish + right_polish
        return self.hash
    
    def get_c_name(self):
        return self.get_polish()
    
    def get_c(self, lookup):
        name = self.get_c_name()
        if name in lookup:
            return ""
        lookup[name] = "d"
        
        left_c = self.left_operand.get_c(lookup)
        right_c = self.right_operand.get_c(lookup)

        c = left_c + right_c

        if self.operation == "/" and "c" not in lookup[self.right_operand.get_c_name()]:
            c += "    %s(%s);\n" % ("NOTZERO", self.right_operand.get_c_name())
            lookup[self.right_operand.get_c_name()] += "c"

        comment = ""
        comment = " // %d bits, %d zeros" % (self.get_bits(), self.get_trailing_zeros())
        c += "    %s(%s,%s,%s);%s\n" % (self.function, name, self.left_operand.get_c_name(), self.right_operand.get_c_name(), comment)

        return c
    
    
    def get_dependent_veriables(self):
        if self.dependent_veriables is None:
            self.dependent_veriables = self.left_operand.get_dependent_veriables().union(self.right_operand.get_dependent_veriables())
        return self.dependent_veriables
    
    def evaluate(self, settings):
        return self.operate(self.left_operand.evaluate(settings), self.right_operand.evaluate(settings))
    
    def get_multiplicands(self):
        return {self.get_polish(): 1}

class Variable(Expression):
    def __init__(self, name: str, value=None):
        super().__init__(None, None)

        if value is None:
            self.possible_values = {1, -1}
        else:
            self.possible_values = {value}
        self.scalar = 1
        self.name = str(name)
        self.is_variable = True

        self.all_factors_are_variables = True

    def evaluate(self, values):
        if len(self.possible_values) == 1:
            return list(self.possible_values)[0] * self.scalar
        return values[self.name]

    def get_string(self): 
        if len(self.possible_values) == 1:
            return "" + str(list(self.possible_values)[0] * self.scalar) + ""
        return "" + ("" if (self.scalar == 1) else '-') + "x" + self.name + ""
    
    def get_polish(self):
        if len(self.possible_values) == 1:
            value = list(self.possible_values)[0]
            scalar = self.scalar
            if value < 0:
                value *= -1
                scalar *= -1
            return ("p" if (scalar == 1) else 'q') + str(value)
        return ("v" if (self.scalar == 1) else 'h') + self.name
    
    def get_c_name(self):
        if len(self.possible_values) == 1:
            value = list(self.possible_values)[0] * self.scalar
            if value == 1:
                return "ONE"
            if value == -1:
                return "NEG_ONE"
            if value == 0:
                return "ZERO"
            print("ERROR THERE IS NO CONSTANT HERE")
            exit(-1)
        return ("x" if (self.scalar == 1) else "n") + self.name
    
    def get_c(self, lookup):
        name = self.get_c_name()
        if name in lookup or self.scalar == 1 or len(self.possible_values) == 1:
            return ""
        lookup[name] = "d"

        return "    NEGATE(" + name + ", " + name.replace("n", "x") +");\n"
    
    def set_value(self, value):
        self.possible_values = {value,}

    def simplify(self):
        return self
    
    def easy_to_negate(self):
        if len(self.possible_values) == 1:
            return True
        return self.scalar == -1
    
    def negate(self):
        if len(self.possible_values) == 1:
            value = list(self.possible_values)[0] * -1
            return Variable(self.name, value)
        else:
            new_variable = Variable(self.name)
            new_variable.possible_values = self.possible_values
            new_variable.scalar = self.scalar * -1
            return new_variable
        
    def get_possible_values_complete(self):
        return self.possible_values
    
    def get_dependent_veriables(self):
        if len(self.possible_values) == 1:
            return set()
        else:
            variables = {self.name}
            return variables
        
    def get_bits(self):
        return 1
    
    def get_trailing_zeros(self):
        return 0
            
class One(Variable):
    def __init__(self):
        super().__init__("one", 1)

    def negate(self):
        return NegativeOne()
    
class NegativeOne(Variable):
    def __init__(self):
        super().__init__("neg_one", -1)

    def negate(self):
        return One()
    
class Multiplication(Expression):
    def __init__(self, left_operand: Expression, right_operand: Expression):
        super().__init__(left_operand, right_operand)
        self.operation = '*'
        self.function = "MUL"
        self.polish_letter = "m"
        self.is_multiplication = True

        self.multiplicands = None
        
    def operate(self, left, right):
        return left * right
    
    def simplify(self):
        super().simplify()

        if self.left_operand.all_factors_are_variables and self.right_operand.all_factors_are_variables:
            self.all_factors_are_variables = True

        right_code = self.right_operand.get_polish()
        left_code = self.left_operand.get_polish()

        if right_code < left_code:
            temp = self.left_operand
            self.left_operand = self.right_operand
            self.right_operand = temp

        if self.left_operand.is_variable:
            possible_left = self.left_operand.possible_values
            if len(possible_left) == 1 and 0 in possible_left:
                return Variable(str(random.random()), 0)
            if len(possible_left) == 1 and 1 in possible_left:
                return self.right_operand
            # if len(possible_left) == 1 and -1 in possible_left:
            #     return self.right_operand.negate()
            
        if self.right_operand.is_variable:
            possible_right = self.right_operand.possible_values
            if len(possible_right) == 1 and 0 in possible_right:
                return Variable(str(random.random()), 0)
            if len(possible_right) == 1 and 1 in possible_right:
                return self.left_operand
            # if len(possible_right) == 1 and -1 in possible_right:
            #     return self.left_operand.negate()
            
        if self.right_operand.is_variable and self.left_operand.is_variable:
            possible_left = self.left_operand.possible_values
            possible_right = self.right_operand.possible_values
            if self.right_operand.name == self.left_operand.name \
                and len(possible_right) == 2 and 1 in possible_right and -1 in possible_right \
                and len(possible_left) == 2  and 1 in possible_left and -1 in possible_left:
                if self.right_operand.scalar == self.left_operand.scalar:
                    return One()
                else:
                    return NegativeOne()
                
        if self.right_operand.is_multiplication and self.left_operand.is_multiplication:
            right_right_code = self.right_operand.right_operand.get_polish()
            right_left_code = self.right_operand.left_operand.get_polish()
            left_left_code = self.left_operand.left_operand.get_polish()
            left_right_code = self.left_operand.right_operand.get_polish()

            if right_right_code != right_left_code and left_right_code != left_left_code:
                if right_right_code == left_right_code:
                    return Multiplication(Multiplication(self.right_operand.right_operand, self.left_operand.right_operand),
                                        Multiplication(self.right_operand.left_operand, self.left_operand.left_operand)).simplify()
                if right_right_code == left_left_code:
                    return Multiplication(Multiplication(self.right_operand.right_operand, self.left_operand.left_operand),
                                        Multiplication(self.right_operand.left_operand, self.left_operand.right_operand)).simplify()
                if right_left_code == left_right_code:
                    return Multiplication(Multiplication(self.right_operand.left_operand, self.left_operand.right_operand),
                                        Multiplication(self.right_operand.right_operand, self.left_operand.left_operand)).simplify()
                if right_left_code == left_left_code:
                    return Multiplication(Multiplication(self.right_operand.left_operand, self.left_operand.left_operand),
                                        Multiplication(self.right_operand.right_operand, self.left_operand.right_operand)).simplify()

        if self.left_operand.is_division and not self.right_operand.is_division:
            return Division(Multiplication(self.left_operand.left_operand, self.right_operand), self.left_operand.right_operand).simplify()
        if self.right_operand.is_division and not self.left_operand.is_division:
            return Division(Multiplication(self.right_operand.left_operand, self.left_operand), self.right_operand.right_operand).simplify()
        if self.right_operand.is_division and self.left_operand.is_division:
            return Division(Multiplication(self.right_operand.left_operand, self.left_operand.left_operand), 
                Multiplication(self.right_operand.right_operand, self.left_operand.right_operand)).simplify()

        return self
    
    def easy_to_negate(self):
        return self.left_operand.easy_to_negate() or self.right_operand.easy_to_negate()
    
    def negate(self):
        if self.right_operand.easy_to_negate():
            return Multiplication(self.left_operand, self.right_operand.negate())
        else:
            return Multiplication(self.left_operand.negate(), self.right_operand)
        
    def get_multiplicands(self):
        if self.multiplicands is None:
            multiplicands = self.left_operand.get_multiplicands()
            
            right_multiplicands = self.right_operand.get_multiplicands()
            for right_multiplicand in right_multiplicands:
                if right_multiplicand not in multiplicands:
                    multiplicands[right_multiplicand] = 0
                multiplicands[right_multiplicand] += right_multiplicands[right_multiplicand]

            self.multiplicands = multiplicands

        return self.multiplicands.copy()
    
    def remove_multiplicands(self, multiplicands):

        some_remain = False
        for m in multiplicands:
            if multiplicands[m] > 0:
                some_remain = True
                break

        if not some_remain:
            return self

        own_multiplicands = self.get_multiplicands()

        all_included = True
        some_included = False
        for m in own_multiplicands:
            if m in multiplicands and multiplicands[m] > 0:
                some_included = True
            if m not in multiplicands or multiplicands[m] < own_multiplicands[m]:
                all_included = False

        if not some_included:
            return self
        
        if all_included:
            for m in own_multiplicands:
                multiplicands[m] -= own_multiplicands[m]

                if multiplicands[m] < 0:
                    print("Error we can't have negative", multiplicands[m])
                    exit(-1)

            return One()

        left_operand = self.left_operand
        right_operand = self.right_operand

        left = left_operand.get_polish()
        right = right_operand.get_polish()

        if left in multiplicands and multiplicands[left] > 0:
            multiplicands[left] -= 1
            if right in multiplicands and multiplicands[right] > 0:
                multiplicands[right] -= 1
                return One()
            else:
                if right_operand.is_multiplication:
                    return right_operand.remove_multiplicands(multiplicands)
                else:
                    return right_operand

        if right in multiplicands and multiplicands[right] > 0:
            multiplicands[right] -= 1
            if left_operand.is_multiplication:
                return left_operand.remove_multiplicands(multiplicands)
            else:
                return left_operand
        
        if left_operand.is_multiplication:
            left_operand = left_operand.remove_multiplicands(multiplicands)
        if right_operand.is_multiplication:
            right_operand = right_operand.remove_multiplicands(multiplicands)
        
        return Multiplication(left_operand, right_operand).simplify()
    
    def get_bits(self):
        if self.bits is None:
            if self.left_operand.get_bits() == 1:
                return self.right_operand.get_bits()
            if self.right_operand.get_bits() == 1:
                return self.left_operand.get_bits()
            self.bits = self.left_operand.get_bits() + self.right_operand.get_bits()
        return self.bits
    
    def get_trailing_zeros(self):
        if self.trailing_zeros is None:
            self.trailing_zeros = self.left_operand.get_trailing_zeros() + self.right_operand.get_trailing_zeros()
        return self.trailing_zeros
            

class Addition(Expression):
    def __init__(self, left_operand: Expression, right_operand: Expression):
        super().__init__(left_operand, right_operand)
        self.operation = "+"
        self.function = "ADD"
        self.polish_letter = "a"
        self.is_addition = True
        
    def operate(self, left, right):
        return left + right
    
    def simplify(self):
        super().simplify()

        right_code = self.right_operand.get_polish()
        left_code = self.left_operand.get_polish()

        if right_code < left_code:
            temp = self.left_operand
            self.left_operand = self.right_operand
            self.right_operand = temp

        if self.left_operand.is_variable:
            possible_left = self.left_operand.possible_values   
            if len(possible_left) == 1 and 0 in possible_left:
                return self.right_operand
            
        if self.right_operand.is_variable:
            possible_right = self.right_operand.possible_values   
            if len(possible_right) == 1 and 0 in possible_right:
                return self.left_operand

        return self
        
    
    def easy_to_negate(self):
        return self.left_operand.easy_to_negate() and self.right_operand.easy_to_negate()
    
    def negate(self):
        return Addition(self.left_operand.negate(), self.right_operand.negate())
    
    def get_bits(self):
        if self.bits is None:
            self.bits = max(self.left_operand.get_bits(), self.right_operand.get_bits()) + 1
        return self.bits
    
    def get_trailing_zeros(self):
        if self.trailing_zeros is None:
            if self.left_operand.all_factors_are_variables and self.right_operand_operand.all_factors_are_variables:
                self.trailing_zeros = 1
            else:
                self.trailing_zeros = min(self.left_operand.get_trailing_zeros(), self.right_operand.get_trailing_zeros())
        return self.trailing_zeros
    
class Condition(Expression):
    def __init__(self, left_operand: Expression, right_operand: Expression):
        super().__init__(left_operand, right_operand)
        self.polish_letter = "q"
        self.is_condition = True

        self.always_true = False
        self.always_false = False
        
    def operate(self, left, right):
        return left == 0 and right != 0
    
    def simplify(self):
        super().simplify()

        if self.left_operand.is_variable and self.right_operand.is_variable:
            possible_left = self.left_operand.possible_values
            possible_right = self.right_operand.possible_values   
            if len(possible_left) == 1 and 0 in possible_left and 0 not in possible_right:
                self.always_true = True
            if 0 not in possible_left or (len(possible_right) == 1 and 0 in possible_right):
                self.always_false = True

        if self.left_operand.is_division:
            return Condition(self.left_operand.left_operand, self.right_operand).simplify()

        if self.right_operand.is_division:
            return Condition(self.left_operand, self.right_operand.left_operand).simplify()

        return self
    
    def get_string(self):
        return "(" + self.left_operand.get_string() + "==0 and " + self.right_operand.get_string() + "!=0)"
    
    def get_c(self, lookup):
        name = self.get_c_name()
        if name in lookup:
            return ""
        lookup[name] = "d"
        
        left_c = self.left_operand.get_c(lookup)
        right_c = self.right_operand.get_c(lookup)

        c = left_c + right_c

        comment = ""
        comment = " // %d bits, %d zeros" % (self.get_bits(), self.get_trailing_zeros())
        c += "    %s(%s,%s,%s);%s\n" % ("TEST_ZERO", name, self.left_operand.get_c_name(), self.right_operand.get_c_name(), comment)
        return c
    
    def get_bits(self):
        return 1
    
    def get_trailing_zeros(self):
        return 0
    

class ConditionalAddition(Expression):
    def __init__(self, left_operand: Expression, right_operand: Expression, condition: Condition):
        super().__init__(left_operand, right_operand)
        self.polish_letter = "c"
        self.is_addition = True
        self.condition = condition

    def evaluate(self, values):
        left_result = self.left_operand.evaluate(values)
        right_result = self.right_operand.evaluate(values)
        condition_result = self.condition.evaluate(values)

        return self.operate(left_result, right_result, condition_result)
        
    def operate(self, left, right, condition):
        if condition:
            return left + right
        else:
            return left
    
    def simplify(self):
        super().simplify()
        self.condition = self.condition.simplify()

        if self.condition.always_true:
            return Addition(self.left_operand, self.right_operand).simplify()
        if self.condition.always_false:
            return self.left_operand
        
        if self.right_operand.is_variable:
            possible_right = self.right_operand.possible_values   
            if len(possible_right) == 1 and 0 in possible_right:
                return self.left_operand
            
        if self.left_operand.is_division and not self.right_operand.is_division:
            return Division(ConditionalAddition(self.left_operand.left_operand, 
                            Multiplication(self.right_operand, self.left_operand.right_operand), self.condition), self.left_operand.right_operand).simplify()
        
        if not self.left_operand.is_division and self.right_operand.is_division:
            return Division(ConditionalAddition(Multiplication(self.left_operand, self.right_operand.right_operand), 
                            self.right_operand.left_operand, self.condition), self.right_operand.right_operand).simplify()
    
        if self.left_operand.is_division and self.right_operand.is_division:
            if self.left_operand.right_operand.get_polish() == self.right_operand.right_operand.get_polish():
                return Division(ConditionalAddition(self.left_operand.left_operand, self.right_operand.left_operand, self.condition), self.left_operand.right_operand).simplify()

            left_multiplicands = self.left_operand.get_multiplicands()
            right_multiplicands = self.right_operand.get_multiplicands()

            similar_factors = {}
            for m in left_multiplicands:
                    if m in right_multiplicands:
                        similar_factors[m] = min(left_multiplicands[m], right_multiplicands[m])

            if len(similar_factors) > 0:
                print("ConditionalAddition of fractions")
                print(len(similar_factors))
            
            return Division(ConditionalAddition(Multiplication(self.left_operand.left_operand, self.right_operand.right_operand), 
                            Multiplication(self.right_operand.left_operand, self.left_operand.right_operand), self.condition), 
                            Multiplication(self.left_operand.right_operand, self.right_operand.right_operand)).simplify()

        return self
    
    def get_string(self):
        return "(" + self.left_operand.get_string() + " + (" + self.condition.get_string() + "?" + self.right_operand.get_string() + ":0))"
    
    def get_polish(self):
        if self.hash is None:
            left_polish = self.left_operand.get_polish()
            right_polish = self.right_operand.get_polish()
            condition_polish = self.condition.get_polish()
            if len(left_polish) > self.c_variable_length:
                left_polish = hex(abs(hash(left_polish)))[2:]
            if len(right_polish) > self.c_variable_length:
                right_polish = hex(abs(hash(right_polish)))[2:]
            if len(condition_polish) > self.c_variable_length:
                condition_polish = hex(abs(hash(condition_polish)))[2:]
            self.hash = self.polish_letter + condition_polish + left_polish + right_polish
        return self.hash
    
    def get_c(self, lookup):
        name = self.get_c_name()
        if name in lookup:
            return ""
        lookup[name] = "d"
        
        left_c = self.left_operand.get_c(lookup)
        right_c = self.right_operand.get_c(lookup)
        condition_c = self.condition.get_c(lookup)

        c = left_c + right_c + condition_c

        comment = ""
        comment = " // %d bits, %d zeros" % (self.get_bits(), self.get_trailing_zeros())
        c += "    %s(%s,%s,%s,%s);%s\n" % ("IFADD", self.condition.get_c_name(), name, self.left_operand.get_c_name(), self.right_operand.get_c_name(), comment)

        return c
    
    def easy_to_negate(self):
        return self.left_operand.easy_to_negate() and self.right_operand.easy_to_negate()
    
    def negate(self):
        return ConditionalAddition(self.left_operand.negate(), self.right_operand.negate(), self.condition)
    
    def get_dependent_veriables(self):
        if self.dependent_veriables is None:
            self.dependent_veriables = self.left_operand.get_dependent_veriables().union(self.right_operand.get_dependent_veriables())
            self.dependent_veriables = self.dependent_veriables.union(self.condition.get_dependent_veriables())
        return self.dependent_veriables
    
    def get_bits(self):
        if self.bits is None:
            self.bits = max(self.left_operand.get_bits(), self.right_operand.get_bits()) + 1
        return self.bits
    
    def get_trailing_zeros(self):
        if self.trailing_zeros is None:
            self.trailing_zeros = min(self.left_operand.get_trailing_zeros(), self.right_operand.get_trailing_zeros())
        return self.trailing_zeros
    
        

class Subtraction(Expression):
    def __init__(self, left_operand: Expression, right_operand: Expression):
        super().__init__(left_operand, right_operand)
        self.operation = '-'
        self.function = "SUB"
        self.polish_letter = "s"
        self.is_subtration = True
    
    def operate(self, left, right):
        return left - right
    
    def simplify(self):
        super().simplify()

        if self.left_operand.is_variable:
            possible_left = self.left_operand.possible_values  
            if len(possible_left) == 1 and 0 in possible_left:
                return self.right_operand.negate()
                
        if self.right_operand.is_variable:
            possible_right = self.right_operand.possible_values   
            if len(possible_right) == 1 and 0 in possible_right:
                return self.left_operand 

        if self.left_operand.get_polish() == self.right_operand.get_polish():
            return Variable(str(random.random()), 0)
        
        if self.left_operand.is_division and not self.right_operand.is_division:
            return Division(Subtraction(self.left_operand.left_operand, 
                            Multiplication(self.right_operand, self.left_operand.right_operand)), self.left_operand.right_operand).simplify()
        
        if not self.left_operand.is_division and self.right_operand.is_division:
            return Division(Subtraction(Multiplication(self.left_operand, self.right_operand.right_operand), 
                            self.right_operand.left_operand), self.right_operand.right_operand).simplify()
    
        if self.left_operand.is_division and self.right_operand.is_division:
            if self.left_operand.right_operand.get_polish() == self.right_operand.right_operand.get_polish():
                print("Easy subtract")
                return Division(Subtraction(self.left_operand.left_operand, self.right_operand.left_operand), self.left_operand.right_operand).simplify()
            

            left_multiplicands = self.left_operand.get_multiplicands()
            right_multiplicands = self.right_operand.get_multiplicands()

            similar_factors = {}
            for m in left_multiplicands:
                    if m in right_multiplicands:
                        similar_factors[m] = min(left_multiplicands[m], right_multiplicands[m])

            if len(similar_factors) > 0:
                print("fraction subtract")
                print(len(similar_factors))
            
            return Division(Subtraction(Multiplication(self.left_operand.left_operand, self.right_operand.right_operand), 
                            Multiplication(self.right_operand.left_operand, self.left_operand.right_operand)), 
                            Multiplication(self.left_operand.right_operand, self.right_operand.right_operand)).simplify()
            
        
        return self
    
    def easy_to_negate(self):
        return True
    
    def negate(self):
        return Subtraction(self.right_operand, self.left_operand).simplify()
    
    def get_bits(self):
        if self.bits is None:
            self.bits = max(self.left_operand.get_bits(), self.right_operand.get_bits()) + 1
        return self.bits
    
    def get_trailing_zeros(self):
        if self.trailing_zeros is None:
            if self.left_operand.all_factors_are_variables and self.right_operand.all_factors_are_variables:
                self.trailing_zeros = 1
            else:
                self.trailing_zeros = min(self.left_operand.get_trailing_zeros(), self.right_operand.get_trailing_zeros())
        return self.trailing_zeros

class Division(Expression):
    def __init__(self, left_operand: Expression, right_operand: Expression):
        super().__init__(left_operand, right_operand)
        self.operation = '/'
        self.function = "DIV"
        self.polish_letter = "d"
        self.is_division = True

    def operate(self, left, right):
        if right == 0:
            return None
        return left / right
        

    def simplify(self):
        super().simplify()

        if self.left_operand.is_variable:
            possible_left = self.left_operand.possible_values
            if len(possible_left) == 1 and 0 in possible_left:
                return self.left_operand
                
        if self.right_operand.is_variable:
            possible_right = self.right_operand.possible_values   
            if len(possible_right) == 1 and 1 in possible_right:
                return self.left_operand
            if len(possible_right) == 1 and -1 in possible_right:
                return self.left_operand.negate()
            if len(possible_right) == 2 and 1 in possible_right and -1 in possible_right:
                return Multiplication(self.left_operand, self.right_operand).simplify()

        if self.left_operand.get_polish() == self.right_operand.get_polish():
            return One()

        if self.left_operand.is_division and not self.right_operand.is_division:
            return Division(self.left_operand.left_operand, Multiplication(self.left_operand.right_operand, self.right_operand)).simplify()

        if not self.left_operand.is_division and self.right_operand.is_division:
            return Division(Multiplication(self.left_operand, self.right_operand.right_operand), self.right_operand.left_operand).simplify()

        if self.left_operand.is_division and self.right_operand.is_division:
            return Division(Multiplication(self.left_operand.left_operand, self.right_operand.right_operand), 
                Multiplication(self.left_operand.right_operand, self.right_operand.left_operand)).simplify()
        
        return self.reduce()
        return self
    
    def reduce(self):
        if self.left_operand.is_multiplication and self.right_operand.is_multiplication:
            left_multiplicands = self.left_operand.get_multiplicands()
            right_multiplicands = self.right_operand.get_multiplicands()

            common_multiplicands = {}
            for m in left_multiplicands:
                if m in right_multiplicands:
                    common_multiplicands[m] = min(left_multiplicands[m], right_multiplicands[m])

            if len(common_multiplicands) > 0:
                # print("left", left_multiplicands)
                # print("right", right_multiplicands)
                # print("common", common_multiplicands)

                left_operand = self.left_operand.remove_multiplicands(common_multiplicands.copy())
                right_operand = self.right_operand.remove_multiplicands(common_multiplicands.copy())


                stil_left_multiplicands = left_operand.get_multiplicands()
                stil_right_multiplicands = right_operand.get_multiplicands()
                still_common_multiplicands = {}
                for m in stil_left_multiplicands:
                    if m in stil_right_multiplicands:
                        still_common_multiplicands[m] = min(stil_left_multiplicands[m], stil_right_multiplicands[m])

                if(len(still_common_multiplicands) > 0):
                    # print("still left", stil_left_multiplicands)
                    # print("still right", stil_right_multiplicands)
                    # print("still common", still_common_multiplicands)
                    print("error", "first had %d, now have %d" % (len(common_multiplicands), len(still_common_multiplicands)))
                    exit(-1)

                return Division(left_operand, right_operand).simplify()
            
        if self.left_operand.is_multiplication and not self.right_operand.is_multiplication:
            right = self.right_operand.get_polish()
            left_multiplicands = self.left_operand.get_multiplicands()
            if right in left_multiplicands:
                return self.left_operand.remove_multiplicands({right: 1})
            
        if not self.left_operand.is_multiplication and self.right_operand.is_multiplication:
            left = self.right_operand.get_polish()
            right_multiplicands = self.left_operand.get_multiplicands()
            if left in right_multiplicands:
                return Division(One(), self.right_operand.remove_multiplicand({left: 1}))

        return self
    
    def easy_to_negate(self):
        return self.left_operand.easy_to_negate() or self.right_operand.easy_to_negate()
    
    def negate(self):
        if self.right_operand.easy_to_negate():
            return Division(self.left_operand, self.right_operand.negate())
        else:
            return Division(self.left_operand.negate(), self.right_operand)


    def evaluate(self, settings):
        left_operand_value = self.left_operand.evaluate(settings)
        right_operand_value = self.right_operand.evaluate(settings)
        if right_operand_value == 0:
            return math.nan
        
        return left_operand_value / right_operand_value
    
    def get_bits(self):
        if self.bits is None:
            self.bits = self.left_operand.get_bits() + self.right_operand.get_bits()
        return self.bits
    
    def get_trailing_zeros(self):
        return 0
