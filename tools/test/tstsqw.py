import numpy as np

a = 12.3
b = np.array([12., 34., 56.])
str = "Test"

def TakinSqw(h,k,l,E):
	print("in tstsqw.py: in Sqw")
	return h*k*l*E + a

print("in tstsqw.py: loaded module")
