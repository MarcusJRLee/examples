import math


h = 6.62607004*pow(10, -34) # J*s
c = 2.99792458*pow(10, 8) # m/s
pi = math.pi
h_bar = h/(2*pi)
m = 9.10938356*pow(10, -31) # kg, mass of electron
nm = 1*pow(10, -9) # 1 nano meter


def E(n, a):
	# Energy of electron is state n at infinite potential well length a (meters)
	num = pow(n, 2)*pow(pi, 2)*pow(h_bar, 2)
	denom = 2*m*pow(a, 2)
	return num/denom

def get_wlen(n1, n2, a):
	# Going from energy level n1 to energy level n2, what photon wavelength is emitted
	e1 = E(n1, a)
	e2 = E(n2, a)
	return (c*h)/(e1 - e2)

def get_a(n1, n2, wlen):
	return pow((wlen*h*(pow(n1, 2) - pow(n2, 2)))/(8*c*m), 0.5)

def m_to_nm(m):
	return m*pow(10, 9)

def E_for_wlen(wlen):
	# wlen in meters
	# Energy in J
	return (h*c)/(wlen)



n1 = 2
n2 = 1
a = 0.1 # nm
wlen = 400 # nm
# print("Wavelength in nano meters of electron moving from n = {0} to n = {1} with a = {2}nm: {3}".format(n1, n2, a, m_to_nm(get_wlen(n1, n2, a*nm))))

# print("Here is a in nano meters for wavelength = {0}nm, from n = {1} to n = {2}: {3}".format(wlen, n1, n2, m_to_nm(get_a(n1, n2, wlen*nm))))

def print_wavelengths(a, topn):
	a = m_to_nm(a) # nm
	for i in range(topn):
		currn = i + 1
		for j in range(i):
			bottomn = j + 1
			wlen = m_to_nm(get_wlen(currn, bottomn, a*nm)) # nm
			energy = E_for_wlen(wlen*nm) # J
			#print("Wavelength in nano meters of electron moving from n = {0} to n = {1} with a = {2}nm: {3}; Energy of photon: {4}".format(currn, bottomn, a, wlen, energy))
			if (j + 1 == i) or (i + 1 == topn and j == 0):
				print("Wavelength in nano meters of electron moving from n = {0} to n = {1} with a = {2}nm: {3}; Energy of photon: {4}".format(currn, bottomn, a, wlen, energy))
				bab = 2

a = m_to_nm(get_a(n1, n2, wlen*nm)) # nm
a = a*5
a = 0.99 # Soln
# a = 1.2
a = 1.6
topn = 10
# print_wavelengths(a*nm, topn)



def print_wavelengths(a, topn):
	a = m_to_nm(a) # nm
	for i in range(topn):
		currn = i + 1
		for j in range(i):
			bottomn = j + 1
			wlen = m_to_nm(get_wlen(currn, bottomn, a*nm)) # nm
			energy = E_for_wlen(wlen*nm) # J
			print("Wavelength in nano meters of electron moving from n = {0} to n = {1} with a = {2}nm: {3}; Energy of photon: {4}".format(currn, bottomn, a, wlen, energy))


#### Part b ####




def E2(n, k):
	# Energy of electron is state n at infinite potential well length a (meters)
	omega = pow(k/m, 2)
	return (n + 0.5)*h_bar*omega

def get_wlen2(n1, n2, k):
	# Going from energy level n1 to energy level n2, what photon wavelength is emitted
	e1 = E2(n1, k)
	e2 = E2(n2, k)
	return (c*h)/(e1 - e2)


def print_wavelengths2(k, topn):
	for i in range(topn):
		currn = i
		for j in range(i):
			bottomn = j
			wlen = m_to_nm(get_wlen2(currn, bottomn, k)) # nm
			energy = E_for_wlen(wlen*nm) # J
			# print("Wavelength in nano meters of electron moving from n = {0} to n = {1} with a = {2}nm: {3}; Energy of photon: {4}".format(currn, bottomn, a, wlen, energy))
			if i + 1 == topn:
				print("Wavelength in nano meters of electron moving from n = {0} to n = {1} with k = {2}J/m^2: {3}; Energy of photon: {4}".format(currn, bottomn, k, wlen, energy))
				bab = 2

k = 0.00001855*nm*nm
k = 1.855*pow(10, -23)
topn = 30
print_wavelengths2(k, topn)

def val(x):
	return 4.22*pow(10, -30)*pow(x, 3) + 2.3575*pow(10, 11)*x + 5.399*pow(10, 26)*math.log(x)

a = 1.5*pow(10, 11)
b = 5*9.46*pow(10, 18)

# print(val(a) - val(b))





















