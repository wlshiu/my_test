MOVC R0,#0
MOVC R1,#1
MOVC R2,#2
MOVC R4,#4
MOVC R5,#1
MOVC R6,#1000
MOVC R7,#4028
BZ #28
MUL R3,R2,R2
STR R3,R6,R0
ADD R6,R6,R4
ADD R2,R2,R1
SUB R5,R5,R1
JUMP R7,#0
HALT